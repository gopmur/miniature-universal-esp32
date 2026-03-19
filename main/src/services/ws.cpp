#include "services/ws.hpp"
#include <optional>
#include "config.hpp"
#include "context/monitoring_data.hpp"
#include "context/services/http.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "helper/json.hpp"
#include "portmacro.h"
#include "service.hpp"
#include "services/stm_uart/rssp.hpp"

WebSocketService::WebSocketService(int priority)
    : AbstractService(priority, "ws"),
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

void WebSocketService::enable_motor_data() {
  this->motor_data_enabled = true;
}
void WebSocketService::disable_motor_data() {
  this->motor_data_enabled = false;
}

bool WebSocketService::has_connections() {
  return connection_count > 0;
}

bool WebSocketService::should_wait_for_eoc() {
  return this->imu_data_enabled || this->stm_cpu_usage_enabled ||
         this->motor_data_enabled;
}

void WebSocketService::fill_esp_cpu_usage_json(JsonObject* esp_cpu_usage_json) {
  esp_cpu_usage_json->set_number("dns", monitoring_data.dns_service_cpu_usage);
  esp_cpu_usage_json->set_number("led", monitoring_data.led_service_cpu_usage);
  esp_cpu_usage_json->set_number("monitor",
                                 monitoring_data.monitor_service_cpu_usage);
  esp_cpu_usage_json->set_number("uartRx",
                                 monitoring_data.stm_uart_rx_service_cpu_usage);
  esp_cpu_usage_json->set_number("ws", monitoring_data.ws_service_cpu_usage);
}

void WebSocketService::fill_json_with_packet_data(
    RsspPacket packet,
    JsonObject* stm_cpu_usage_json,
    JsonObject* imu_data_json,
    JsonObject* motor_data_json) {
  if (this->stm_cpu_usage_enabled) {
    switch (packet.address) {
      case RsspAddress::LED_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("led");
        auto child_object_result = stm_cpu_usage_json->get_object("led");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::IMU_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("imu");
        auto child_object_result = stm_cpu_usage_json->get_object("imu");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }

      case RsspAddress::MOTOR_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("motor");
        auto child_object_result = stm_cpu_usage_json->get_object("motor");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::SD_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("sd");
        auto child_object_result = stm_cpu_usage_json->get_object("sd");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::CAN_RECV_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("canRecv");
        auto child_object_result = stm_cpu_usage_json->get_object("canRecv");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::ESP_UART_RX_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("uartRx");
        auto child_object_result = stm_cpu_usage_json->get_object("uartRx");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::ESP_UART_TX_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("uartTx");
        auto child_object_result = stm_cpu_usage_json->get_object("uartTx");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::MONITOR_SERVICE_CPU_USAGE: {
        stm_cpu_usage_json->add_object("monitor");
        auto child_object_result = stm_cpu_usage_json->get_object("monitor");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("cpuUsage", packet.get_float());
        break;
      }
      case RsspAddress::LED_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("led");
        auto child_object_result = stm_cpu_usage_json->get_object("led");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::IMU_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("imu");
        auto child_object_result = stm_cpu_usage_json->get_object("imu");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }

      case RsspAddress::MOTOR_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("motor");
        auto child_object_result = stm_cpu_usage_json->get_object("motor");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::SD_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("sd");
        auto child_object_result = stm_cpu_usage_json->get_object("sd");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::CAN_RECV_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("canRecv");
        auto child_object_result = stm_cpu_usage_json->get_object("canRecv");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::ESP_UART_RX_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("uartRx");
        auto child_object_result = stm_cpu_usage_json->get_object("uartRx");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::ESP_UART_TX_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("uartTx");
        auto child_object_result = stm_cpu_usage_json->get_object("uartTx");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      case RsspAddress::MONITOR_SERVICE_MAX_STACK_USAGE: {
        stm_cpu_usage_json->add_object("monitor");
        auto child_object_result = stm_cpu_usage_json->get_object("monitor");
        auto child_object = std::get<JsonObject>(child_object_result);
        child_object.set_number("maxStackUsage", packet.get_uint32());
        break;
      }
      default:
        break;
    }
  }
  if (this->imu_data_enabled) {
    switch (packet.address) {
      case RsspAddress::IMU_GX:
        imu_data_json->set_number("x", packet.get_float());
        break;
      case RsspAddress::IMU_GY:
        imu_data_json->set_number("y", packet.get_float());
        break;
      case RsspAddress::IMU_GZ:
        imu_data_json->set_number("z", packet.get_float());
        break;
      default:
        break;
    }
  }
  if (this->motor_data_enabled) {
    switch (packet.address) {
      case RsspAddress::MOTOR_POS_LEFT:
        motor_data_json->set_number("leftPosition", packet.get_uint16());
        break;
      case RsspAddress::MOTOR_POS_RIGHT:
        motor_data_json->set_number("rightPosition", packet.get_uint16());
        break;
      default:
        break;
    }
  }
}

void WebSocketService::fill_root_json(JsonObject* json,
                                      JsonObject* stm_cpu_usage_json,
                                      JsonObject* esp_cpu_usage_json,
                                      JsonObject* imu_data_json,
                                      JsonObject* motor_data_json) {
  if (this->stm_cpu_usage_enabled)
    json->set_object("stmCpuUsage", stm_cpu_usage_json);
  if (this->esp_cpu_usage_enabled)
    json->set_object("espCpuUsage", esp_cpu_usage_json);
  if (this->imu_data_enabled)
    json->set_object("imu", imu_data_json);
  if (this->motor_data_enabled)
    json->set_object("motor", motor_data_json);
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

void WebSocketService::main() {
  JsonObject json;
  JsonObject esp_cpu_usage_json;
  JsonObject stm_cpu_usage_json;
  JsonObject imu_data_json;
  JsonObject motor_data_json;

  while (true) {
    this->wait_for_notification();

    while (true) {
      uint64_t microseconds = esp_timer_get_time();
      if (!this->has_connections()) {
        break;
      }
      if (this->should_wait_for_eoc()) {
        auto packet = this->queue.receive(100);
        if (!packet.has_value())
          continue;
        if (packet->header.b.type == RsspType::EOC) {
          this->queue.flush();
          if (this->esp_cpu_usage_enabled) {
            this->fill_esp_cpu_usage_json(&esp_cpu_usage_json);
          }
          this->fill_root_json(&json,
                               &stm_cpu_usage_json,
                               &esp_cpu_usage_json,
                               &imu_data_json,
                               &motor_data_json);
          if (!json.is_empty()) {
            json.set_number("microseconds", microseconds);
            char* json_str = json.stringify();
            this->send_to_connections(json_str);
            json = JsonObject();
            esp_cpu_usage_json = JsonObject();
            stm_cpu_usage_json = JsonObject();
            imu_data_json = JsonObject();
            free(json_str);
          }

        } else {
          this->fill_json_with_packet_data(packet.value(),
                                           &stm_cpu_usage_json,
                                           &imu_data_json,
                                           &motor_data_json);
        }

      }

      else {
        this->fill_esp_cpu_usage_json(&esp_cpu_usage_json);
        this->fill_root_json(&json,
                             &stm_cpu_usage_json,
                             &esp_cpu_usage_json,
                             &imu_data_json,
                             &motor_data_json);
        if (!json.is_empty()) {
          json.set_number("microseconds", microseconds);
          char* json_str = json.stringify();
          json = JsonObject();
          esp_cpu_usage_json = JsonObject();
          stm_cpu_usage_json = JsonObject();
          imu_data_json = JsonObject();
          this->send_to_connections(json_str);
          free(json_str);
        }
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

bool WebSocketService::uart_streams_enabled() {
  return this->imu_data_enabled || this->motor_data_enabled ||
         this->stm_cpu_usage_enabled;
}