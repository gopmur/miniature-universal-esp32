
#include "http/modules/wifi.hpp"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper/formats.hpp"
#include "http/module.hpp"
#include "jayson.hpp"

HttpWifiModule::PutConnectionAsyncHandler HttpWifiModule::put_connect;
HttpWifiModule::GetScanAsyncHandler HttpWifiModule::get_scan;

void HttpWifiModule::GetScanAsyncHandler::main(httpd_req_t** req_p) {
  auto req = *req_p;
  wifi_scan_config_t scan_config = {
      .ssid = nullptr,
      .bssid = nullptr,
      .channel = 0,
      .show_hidden = false,
      .scan_type = WIFI_SCAN_TYPE_ACTIVE,
      .scan_time =
          {
              .active =
                  {
                      .min = 30,
                      .max = 120,
                  },
              .passive = 500,
          },
      .home_chan_dwell_time = 0,
      .channel_bitmap =
          {
              .ghz_2_channels = 0,
              .ghz_5_channels = 0,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_scan_stop());
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

  uint16_t ap_count = 0;
  esp_wifi_scan_get_ap_num(&ap_count);

  wifi_ap_record_t* ap_records = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * ap_count);

  esp_wifi_scan_get_ap_records(&ap_count, ap_records);

  JsonArray root_json;
  for (int i = 0; i < ap_count; i++) {
    JsonObject ap_json;
    ap_json.set("rssi", ap_records[i].rssi);
    auto bssid = get_bssid_string(ap_records[i].bssid);
    ap_json.set("bssid", bssid.get_data());
    ap_json.set("ssid", reinterpret_cast<char*>(ap_records[i].ssid));
    ap_json.set("open", ap_records[i].authmode == WIFI_AUTH_OPEN);
    root_json.append_object(&ap_json);
  }
  send_json(req, root_json);
  free(ap_records);
  httpd_req_async_handler_complete(req);
}

void HttpWifiModule::PutConnectionAsyncHandler::main(HttpJsonAsyncHandlerArgs* args) {
  JsonObject error_json;
  auto req_json = args->json;
  auto req = args->req;
  auto ssid_result = req_json.get_string("ssid", &error_json);
  auto password_result = req_json.get_string("password", &error_json);
  wifi_config_t sta_config = {};

  if (std::holds_alternative<char*>(ssid_result)) {
    auto ssid = std::get<char*>(ssid_result);
    int ssid_len = strlen(ssid);
    if (ssid_len >= 32) {
      error_json.set("ssid", "too long");
    } else {
      std::strcpy(reinterpret_cast<char*>(sta_config.sta.ssid), ssid);
    }
  }

  if (std::holds_alternative<char*>(password_result)) {
    auto password = std::get<char*>(password_result);
    int password_len = strlen(password);
    if (password_len >= 32) {
      error_json.set("password", "too long");
    } else {
      std::strcpy(reinterpret_cast<char*>(sta_config.sta.password), password);
    }
  }

  if (!error_json.is_empty()) {
    send_json(req, error_json, HTTPD_400_BAD_REQUEST);
  }

  else {
    connection_result_queue.flush();
    sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    auto wifi_connection_result = connection_result_queue.receive();
    JsonObject resp_json;
    switch (wifi_connection_result.value()) {
      case WifiConnectionRequestResult::OK:
        send_success_json(req);
        break;
      case WifiConnectionRequestResult::FAILED: {
        send_message_json(req, "connection failed", HTTPD_400_BAD_REQUEST);
        break;
      }
      case WifiConnectionRequestResult::WRONG_SSID: {
        send_message_json(req, "ap not found", HTTPD_400_BAD_REQUEST);
        break;
      }
      case WifiConnectionRequestResult::OTHER: {
        send_message_json(req, "unhandled error", HTTPD_500_INTERNAL_SERVER_ERROR);
        break;
      }
    }
  }

  httpd_req_async_handler_complete(req);
}

esp_err_t HttpWifiModule::get(httpd_req_t* req) {
  wifi_ap_record_t ap_info;
  auto result = esp_wifi_sta_get_ap_info(&ap_info);
  JsonObject res_json;
  if (result == ESP_ERR_WIFI_NOT_CONNECT) {
    res_json.set("connected", false);
    res_json.set("errorMessage", "not connected");
  } else if (result == ESP_ERR_WIFI_CONN) {
    res_json.set("connected", false);
    res_json.set("errorMessage", "wifi not initialized");
  } else {
    res_json.set("connected", true);
    res_json.set("ssid", reinterpret_cast<char*>(ap_info.ssid));
    res_json.set("rssi", ap_info.rssi);
    auto bssid_str = get_bssid_string(ap_info.bssid);
    res_json.set("bssid", bssid_str.get_data());
  };
  return send_json(req, res_json);
}

esp_err_t HttpWifiModule::get_disconnect(httpd_req_t* req) {
  esp_wifi_disconnect();
  return send_success_json(req);
}

Queue<WifiConnectionRequestResult, 1>* HttpWifiModule::get_connection_result_queue() {
  return &put_connect.connection_result_queue;
}

void HttpWifiModule::register_direct_uris() {
  register_async_uri("/scan", HTTP_GET, &get_scan);
  register_uri("/", HTTP_GET, get);
  register_async_uri_with_option("/connect", HTTP_PUT, &put_connect);
  register_uri("/disconnect", HTTP_GET, get_disconnect);
}
