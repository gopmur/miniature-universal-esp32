#include "services/ws.hpp"
#include "cJSON.h"
#include "config.hpp"
#include "context.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "portmacro.h"
#include "service.hpp"
#include "services/stm_uart/lappl.hpp"

WebSocketService::WebSocketService() {
  this->fd = -1;
};

bool WebSocketService::is_connected() {
  return this->fd >= 0;
}

void WebSocketService::main(WebSocketService* self) {
  cJSON* root = cJSON_CreateObject();
  while (true) {
    self->wait_for_notification();

    while (true) {
      auto packet = self->queue.receive(portMAX_DELAY);
      if (self->fd < 0 || !packet.has_value()) {
        break;
      }
      if (packet->header.b.type == LapplType::EOC) {
        self->queue.flush();
        char* json_str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        httpd_ws_frame_t ws_packet = {
            .final = true,
            .fragmented = false,
            .type = HTTPD_WS_TYPE_TEXT,
            .payload = (uint8_t*)json_str,
            .len = strlen(json_str),
        };
        esp_err_t ret =
            httpd_ws_send_frame_async(context::http_service.server_instance,
                                      self->fd,
                                      &ws_packet);
        if (ret != ESP_OK) {
          ESP_LOGW("WS", "Client disconnected or send failed");
          self->fd = -1;
        }
        free(json_str);
        root = cJSON_CreateObject();
        continue;
      }
      switch (packet->address) {
        case LapplAddress::IMU_GX:
          cJSON_AddNumberToObject(root, "imuGx", packet->get_float());
          break;
        case LapplAddress::IMU_GY:
          cJSON_AddNumberToObject(root, "imuGy", packet->get_float());
          break;
        case LapplAddress::IMU_GZ:
          cJSON_AddNumberToObject(root, "imuGz", packet->get_float());
          break;
        case LapplAddress::LED_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "ledServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::IMU_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "imuServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::MOTOR_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "motorServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::SD_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "sdServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::CAN_RECV_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "canRecvServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::ESP_UART_RX_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "uartEspRxServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::ESP_UART_TX_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "uartEspTxServiceCpuUsage",
                                  packet->get_float());
          break;
        case LapplAddress::MONITOR_SERVICE_CPU_USAGE:
          cJSON_AddNumberToObject(root,
                                  "monitorServiceCpuUsage",
                                  packet->get_float());
          break;
        default:
          break;
      }
    }
  }
}

void WebSocketService::start_sending(int fd) {
  this->fd = fd;
  this->notify();
}

void WebSocketService::stop_sending() {
  this->fd = 0;
}

void WebSocketService::start() {
  START_SERVICE("NAME", config::service::ws::priority);
}