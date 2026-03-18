#include "threads/scan_wifis.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper/json.hpp"

ScanWifisThread::ScanWifisThread(const char* name, int priority, int stack_size)
    : AbstractThread(name, priority, stack_size) {}

void ScanWifisThread::main(httpd_req_t** req_p) {
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

  wifi_ap_record_t* ap_records =
      (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * 20);

  uint16_t number = ap_count;
  if (number > 20)
    number = 20;

  esp_wifi_scan_get_ap_records(&number, ap_records);

  JsonArray root_json;
  for (int i = 0; i < number; i++) {
    JsonObject ap_json;
    ap_json.set_number("rssi", ap_records[i].rssi);
    ap_json.set_string("bssid", reinterpret_cast<char*>(ap_records[i].bssid));
    ap_json.set_string("ssid", reinterpret_cast<char*>(ap_records[i].ssid));
    ap_json.set_bool("open", ap_records[i].authmode == WIFI_AUTH_OPEN);
    root_json.append_object(&ap_json);
  }
  auto res_str = root_json.stringify();

  httpd_resp_send(req, res_str, HTTPD_RESP_USE_STRLEN);
  free(res_str);
  free(ap_records);
  httpd_req_async_handler_complete(req);
}