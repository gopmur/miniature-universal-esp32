#include "services/ws.hpp"
#include <optional>
#include "config.hpp"
#include "context/monitoring_data.hpp"
#include "context/services/http.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "portmacro.h"
#include "service.hpp"
#include "services/stm_uart/lappl.hpp"

WebSocketService::WebSocketService(int priority)
    : AbstractService(priority),
      esp_cpu_usage_enabled(false),
      stm_cpu_usage_enabled(false),
      imu_data_enabled(false) {
  connection_age.fill(-1);
  connection_fds.fill(-1);
  connection_count = 0;
};

void WebSocketService::enable_esp_cpu_usage() {
  this->esp_cpu_usage_enabled = true;
}

void WebSocketService::disable_esp_cpu_usage() {
  this->esp_cpu_usage_enabled = false;
}

void WebSocketService::enable_stm_cpu_usage() {
  this->stm_cpu_usage_enabled = true;
}
void WebSocketService::disable_stm_cpu_usage() {
  this->stm_cpu_usage_enabled = false;
}
void WebSocketService::enable_imu_data() {
  this->imu_data_enabled = true;
}
void WebSocketService::disable_imu_data() {
  this->imu_data_enabled = false;
}

bool WebSocketService::has_connections() {
  return connection_count > 0;
}

bool WebSocketService::should_wait_for_eoc() {
  return this->imu_data_enabled || this->stm_cpu_usage_enabled;
}

void WebSocketService::fill_esp_cpu_usage_json(Json* esp_cpu_usage_json) {
  esp_cpu_usage_json->set_number("dns", monitoring_data.dns_service_cpu_usage);
  esp_cpu_usage_json->set_number("led", monitoring_data.led_service_cpu_usage);
  esp_cpu_usage_json->set_number("monitor",
                                 monitoring_data.monitor_service_cpu_usage);
  esp_cpu_usage_json->set_number("uartRx",
                                 monitoring_data.stm_uart_rx_service_cpu_usage);
  esp_cpu_usage_json->set_number("ws", monitoring_data.ws_service_cpu_usage);
}

void WebSocketService::fill_json_with_packet_data(LapplPacket packet,
                                                  Json* stm_cpu_usage_json,
                                                  Json* imu_data_json) {
  if (this->stm_cpu_usage_enabled) {
    switch (packet.address) {
      case LapplAddress::LED_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("led", packet.get_float());
        break;
      case LapplAddress::IMU_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("imu", packet.get_float());
        break;
      case LapplAddress::MOTOR_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("motor", packet.get_float());
        break;
      case LapplAddress::SD_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("sd", packet.get_float());
        break;
      case LapplAddress::CAN_RECV_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("canRecv", packet.get_float());
        break;
      case LapplAddress::ESP_UART_RX_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("uartRx", packet.get_float());
        break;
      case LapplAddress::ESP_UART_TX_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("uartTx", packet.get_float());
        break;
      case LapplAddress::MONITOR_SERVICE_CPU_USAGE:
        stm_cpu_usage_json->set_number("monitor", packet.get_float());
        break;
      default:
        break;
    }
  }
  if (this->imu_data_enabled) {
    switch (packet.address) {
      case LapplAddress::IMU_GX:
        imu_data_json->set_number("x", packet.get_float());
        break;
      case LapplAddress::IMU_GY:
        imu_data_json->set_number("y", packet.get_float());
        break;
      case LapplAddress::IMU_GZ:
        imu_data_json->set_number("z", packet.get_float());
        break;
      default:
        break;
    }
  }
}

void WebSocketService::fill_root_json(Json* json,
                                      Json* stm_cpu_usage_json,
                                      Json* esp_cpu_usage_json,
                                      Json* imu_data_json) {
  if (this->stm_cpu_usage_enabled)
    json->set_object("stmCpuUsage", stm_cpu_usage_json);
  if (this->esp_cpu_usage_enabled)
    json->set_object("espCpuUsage", esp_cpu_usage_json);
  if (this->imu_data_enabled)
    json->set_object("imu", imu_data_json);
}

void WebSocketService::send_to_connections(const char* data) {
  httpd_ws_frame_t ws_packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = (uint8_t*)data,
      .len = strlen(data),
  };
  for (int i = 0; i < this->connection_count; i++) {
    int fd = this->connection_fds[i];
    esp_err_t ret =
        httpd_ws_send_frame_async(http_service.server_instance, fd, &ws_packet);
    if (ret != ESP_OK) {
      ESP_LOGW("WS", "Client disconnected or send failed");
      this->stop_sending(fd);
    }
  }
}

void WebSocketService::main(WebSocketService* self) {
  Json json;
  Json esp_cpu_usage_json;
  Json stm_cpu_usage_json;
  Json imu_data_json;

  while (true) {
    self->wait_for_notification();

    while (true) {
      if (!self->has_connections()) {
        break;
      }
      if (self->should_wait_for_eoc()) {
        auto packet = self->queue.receive(100);
        if (!packet.has_value())
          continue;
        if (packet->header.b.type == LapplType::EOC) {
          self->queue.flush();
          if (self->esp_cpu_usage_enabled) {
            self->fill_esp_cpu_usage_json(&esp_cpu_usage_json);
          }
          self->fill_root_json(&json,
                               &stm_cpu_usage_json,
                               &esp_cpu_usage_json,
                               &imu_data_json);
          char* json_str = json.stringify();
          json = Json();
          esp_cpu_usage_json = Json();
          stm_cpu_usage_json = Json();
          imu_data_json = Json();

          self->send_to_connections(json_str);
          free(json_str);

        } else {
          self->fill_json_with_packet_data(packet.value(),
                                           &stm_cpu_usage_json,
                                           &imu_data_json);
        }

      }

      else {
        self->fill_esp_cpu_usage_json(&esp_cpu_usage_json);
        self->fill_root_json(&json,
                             &stm_cpu_usage_json,
                             &esp_cpu_usage_json,
                             &imu_data_json);
        char* json_str = json.stringify();
        json = Json();
        esp_cpu_usage_json = Json();
        stm_cpu_usage_json = Json();
        imu_data_json = Json();
        self->send_to_connections(json_str);
        free(json_str);
        vTaskDelay(25);
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
  START_SERVICE("ws_service");
}