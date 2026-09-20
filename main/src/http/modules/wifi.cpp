
#include "http/modules/wifi.hpp"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper/formats.hpp"
#include "helper/json.hpp"
#include "tasks/wifi_con_handler.hpp"

extern WifiConHandlerTask* wifi_con_handler_task;
ScanWifisThread HttpWifiModule::scan_wifis_thread;

esp_err_t HttpWifiModule::get_scan(httpd_req_t* req) {
  set_header(req);
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  scan_wifis_thread.start("ws_service", 2, 4096, async_req);
  return ESP_OK;
}

esp_err_t HttpWifiModule::get(httpd_req_t* req) {
  set_header(req);
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
  auto res_str = res_json.stringify();
  httpd_resp_send(req, res_str.c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t HttpWifiModule::put_connect(httpd_req_t* req) {
  set_header(req);
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  auto service_not_busy = wifi_con_handler_task->req_queue.send(async_req, 0);
  if (!service_not_busy) {
    JsonObject res_json;
    res_json.set("message", "another connection request is pending");
    auto res_str = res_json.stringify();
    httpd_resp_send_err(async_req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
  }
  return ESP_OK;
}
esp_err_t HttpWifiModule::get_disconnect(httpd_req_t* req) {
  set_header(req);
  esp_wifi_disconnect();
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

void HttpWifiModule::register_direct_uris() {
  register_uri("/scan", HTTP_GET, get_scan);
  register_uri("/", HTTP_GET, get);
  register_uri_with_option("/connect", HTTP_PUT, put_connect);
  register_uri("/disconnect", HTTP_GET, get_disconnect);
}