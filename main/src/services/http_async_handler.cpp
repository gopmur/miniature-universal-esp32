#include "services/http_async_handler.hpp"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "helper/json.hpp"
#include "portmacro.h"
#include "service.hpp"

HttpAsyncHandlerService::HttpAsyncHandlerService(int priority)
    : AbstractService(priority) {}

void HttpAsyncHandlerService::main(HttpAsyncHandlerService* self) {
  while (true) {
    auto message = self->queue.receive(portMAX_DELAY);
    if (!message.has_value()) {
      continue;
    }
    auto req = message->req;
    auto type = message->type;
    switch (type) {
      case HttpAsyncHandlerMessageType::SCAN_WIFI: {
        wifi_scan_config_t scan_config = {0};

        esp_wifi_scan_start(&scan_config, true);

        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);

        wifi_ap_record_t* ap_records =
            (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * 20);

        uint16_t number = ap_count;
        if (number > 20)
          number = 20;

        esp_wifi_scan_get_ap_records(&number, ap_records);

        Json root_json;
        for (int i = 0; i < number; i++) {
          Json ap_json;
          ap_json.set_number("rssi", ap_records[i].rssi);
          root_json.set_object(reinterpret_cast<char*>(ap_records[i].ssid),
                               &ap_json);
        }
        auto res_str = root_json.stringify();

        httpd_resp_send(req, res_str, HTTPD_RESP_USE_STRLEN);
        free(res_str);
        free(ap_records);
      }
    }
    httpd_req_async_handler_complete(req);
  }
}

void HttpAsyncHandlerService::start() {
  START_SERVICE("http_async_handler_service")
}
