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
  connection_age.fill(-1);
  connection_fds.fill(-1);
  connection_count = 0;
};

bool WebSocketService::has_connections() {
  return connection_count > 0;
}

void WebSocketService::main(WebSocketService* self) {
  cJSON* root = cJSON_CreateObject();
  while (true) {
    self->wait_for_notification();

    while (true) {
      auto packet = self->queue.receive(portMAX_DELAY);
      if (!self->has_connections() || !packet.has_value()) {
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
        for (int i = 0; i < self->connection_count; i++) {
          int fd = self->connection_fds[i];
          esp_err_t ret =
              httpd_ws_send_frame_async(context::http_service.server_instance,
                                        fd,
                                        &ws_packet);
          if (ret != ESP_OK) {
            ESP_LOGW("WS", "Client disconnected or send failed");
            self->stop_sending(fd);
          }
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
  if (connection_count < config::service::ws::max_connection) {
    this->connection_fds[this->connection_count] = fd;
    for (int i = 0; i < this->connection_count; i++) {
      this->connection_age[i]++;
    }
    this->connection_age[this->connection_count] = 0;
    this->connection_count++;
  } else {
    int max_age_connection = 0;
    int max_age = 0;
    for (int i = 0; i < this->connection_count; i++) {
      if (this->connection_age[i] > max_age) {
        max_age = this->connection_age[i];
        max_age_connection = i;
      }
      this->connection_age[i]++;
    }
    this->connection_age[max_age_connection] = 0;
    this->connection_fds[max_age_connection] = fd;
  }

  this->notify();
}

void WebSocketService::stop_sending(int fd) {
  if (this->connection_count == 0) {
    return;
  }
  int connection_to_remove = -1;
  int connection_to_remove_age = 0;
  for (int i = 0; i < this->connection_count; i++) {
    if (this->connection_fds[i] == fd) {
      connection_to_remove = i;
      connection_to_remove_age = this->connection_age[i];
      break;
    }
  }
  if (connection_to_remove == -1) {
    return;
  }
  for (int i = 0; i < this->connection_count; i++) {
    if (this->connection_age[i] > connection_to_remove_age) {
      this->connection_age[i]--;
    }
  }
  for (int i = connection_to_remove; i < this->connection_count - 1; i++) {
    this->connection_age[i] = this->connection_age[i + 1];
    this->connection_fds[i] = this->connection_fds[i + 1];
  }
  this->connection_count--;

}

void WebSocketService::start() {
  START_SERVICE("NAME", config::service::ws::priority);
}