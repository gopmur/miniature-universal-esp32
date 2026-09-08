#include "callbacks/wifi_event_handler.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "tasks/wifi_con_handler.hpp"

extern WifiConHandlerTask wifi_con_handler_task;

void wifi_event_handler(void* arg,
                        esp_event_base_t event_base,
                        int32_t event_id,
                        void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI("Wifi", "Connection start");
    esp_wifi_connect();
  }

  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*)event_data;
    switch (disconn->reason) {
      case WIFI_REASON_AUTH_FAIL:
      case WIFI_REASON_AUTH_EXPIRE:
      case WIFI_REASON_HANDSHAKE_TIMEOUT:
      case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        wifi_con_handler_task.connection_result_queue.send(WifiConnectionRequestResult::FAILED, 0);
        break;

      case WIFI_REASON_NO_AP_FOUND:
        wifi_con_handler_task.connection_result_queue.send(WifiConnectionRequestResult::WRONG_SSID,
                                                           0);
        break;

      case WIFI_REASON_ASSOC_LEAVE:
        break;

      default:
        wifi_con_handler_task.connection_result_queue.send(WifiConnectionRequestResult::OTHER, 0);
        break;
    }
  }

  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    wifi_con_handler_task.connection_result_queue.send(WifiConnectionRequestResult::OK, 0);
  }
}