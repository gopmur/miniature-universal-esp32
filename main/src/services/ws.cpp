#include "services/ws.hpp"
#include <optional>
#include "config.hpp"
#include "context/services/http.hpp"
#include "context/services/monitor.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "helper/json.hpp"
#include "service.hpp"
#include "services/stm_uart/rssp.hpp"

#include "context/services/dns.hpp"
#include "context/services/led.hpp"
#include "context/services/stm_uart_rx.hpp"
#include "context/services/ws.hpp"

WebSocketService::WebSocketService(int priority) : Service(priority, "ws") {
  connection_age.fill(-1);
  connection_fds.fill(-1);
  connection_count = 0;
  stream_enabled.fill(false);
};

bool WebSocketService::stream_is_enabled(WsStream stream) {
  return this->stream_enabled[static_cast<size_t>(stream)];
}

void WebSocketService::enable_stream(WsStream stream) {
  if (this->stream_is_enabled(stream)) {
    return;
  }
  enabled_stream_count++;
  this->stream_enabled[static_cast<size_t>(stream)] = true;
  if (enabled_stream_count == 1) {
    this->resume();
  }
}
void WebSocketService::disable_stream(WsStream stream) {
  if (!this->stream_is_enabled(stream)) {
    return;
  }
  enabled_stream_count--;
  this->stream_enabled[static_cast<size_t>(stream)] = false;
  if (enabled_stream_count == 0) {
    this->suspend();
  }
}

bool WebSocketService::has_connections() {
  return connection_count > 0;
}

bool WebSocketService::should_wait_for_eoc() {
  return this->stream_enabled[static_cast<size_t>(WsStream::IMU_DATA)] ||
         this->stream_enabled[static_cast<size_t>(WsStream::MOTOR_DATA)] ||
         this->stream_enabled[static_cast<size_t>(WsStream::STM_TASK_DATA)];
}

void WebSocketService::fill_esp_cpu_usage_json(JsonObject* esp_cpu_usage_json) {
  esp_cpu_usage_json->add_object("dns");
  auto service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object("dns"));
  service_object.set_number("cpuUsage", dns_service.get_cpu_usage());
  service_object.set_number("maxStackUsage", dns_service.get_max_stack_usage());

  esp_cpu_usage_json->add_object("led");
  service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object("led"));
  service_object.set_number("cpuUsage", led_service.get_cpu_usage());
  service_object.set_number("maxStackUsage", led_service.get_max_stack_usage());

  esp_cpu_usage_json->add_object("monitor");
  service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object("monitor"));
  service_object.set_number("cpuUsage", monitor_service.get_cpu_usage());
  service_object.set_number("maxStackUsage", monitor_service.get_max_stack_usage());

  esp_cpu_usage_json->add_object("uartRx");
  service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object("uartRx"));
  service_object.set_number("cpuUsage", stm_uart_rx_service.get_cpu_usage());
  service_object.set_number("maxStackUsage", stm_uart_rx_service.get_max_stack_usage());

  esp_cpu_usage_json->add_object("ws");
  service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object("ws"));
  service_object.set_number("cpuUsage", ws_service.get_cpu_usage());
  service_object.set_number("maxStackUsage", ws_service.get_max_stack_usage());
}

void WebSocketService::fill_json_with_packet_data(RsspPacket packet,
                                                  JsonObject* stm_cpu_usage_json,
                                                  JsonObject* imu_data_json,
                                                  JsonObject* motor_data_json) {
  if (this->stream_is_enabled(WsStream::STM_TASK_DATA)) {
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
  if (this->stream_is_enabled(WsStream::IMU_DATA)) {
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
  if (this->stream_is_enabled(WsStream::MOTOR_DATA)) {
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
  if (this->stream_is_enabled(WsStream::STM_TASK_DATA))
    json->set_object("stmCpuUsage", stm_cpu_usage_json);
  if (this->stream_is_enabled(WsStream::ESP_TASK_DATA))
    json->set_object("espCpuUsage", esp_cpu_usage_json);
  if (this->stream_is_enabled(WsStream::IMU_DATA))
    json->set_object("imu", imu_data_json);
  if (this->stream_is_enabled(WsStream::MOTOR_DATA))
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
    esp_err_t ret = httpd_ws_send_frame_async(http_service.server_instance, fd, &ws_packet);
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
    this->suspend();

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
          if (this->stream_is_enabled(WsStream::ESP_TASK_DATA)) {
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
        this->queue.flush();
        ESP_LOGI("WS", "HERE");
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
  if (enabled_stream_count > 0) {
    this->resume();
  }
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
  if (this->connection_count == 0) {
    this->queue.flush();
    this->suspend();
  }
}

bool WebSocketService::uart_streams_enabled() {
  return this->should_wait_for_eoc();
}