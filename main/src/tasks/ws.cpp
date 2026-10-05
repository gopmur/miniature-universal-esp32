#include "tasks/ws.hpp"
#include <format>
#include <vector>

#include "custom_drivers/motor.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "helper/formats.hpp"
#include "http.hpp"
#include "jayson.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/sync.hpp"
#include "system_logger.hpp"
#include "tasks/imu.hpp"
#include "tasks/monitor.hpp"
#include "tasks/motor.hpp"

extern ImuTask* imu_task;
extern HttpServer http_server;
extern MonitorTask* monitor_task;
extern MotorTask* motor_task;

WebSocketTask::WebSocketTask() : connection_mutex(true) {};

void WebSocketTask::add_time_stamp(JsonObject* json) {
  json->set("microseconds", esp_timer_get_time());
}

void WebSocketTask::fill_imu_data_json(JsonObject* imu_data_json) {
  add_time_stamp(imu_data_json);
  imu_data_json->set("x", imu_task->data.gyro.x);
  imu_data_json->set("y", imu_task->data.gyro.y);
  imu_data_json->set("z", imu_task->data.gyro.z);
}

void WebSocketTask::fill_motor_data_json(JsonObject* motor_data_json) {
  add_time_stamp(motor_data_json);
  for (int i = 0; i < motor_task->motor_count; i++) {
    auto position_key = std::format("position{}", i);
    auto velocity_key = std::format("velocity{}", i);
    motor_data_json->set(position_key.c_str(), motor_task->get_position(i));
    motor_data_json->set(velocity_key.c_str(), motor_task->get_velocity(i));
  }
}

JsonObject WebSocketTask::fill_task_status_json(std::vector<ThreadStatus>& thread_status) {
  JsonObject task_status_json;
  add_time_stamp(&task_status_json);
  auto tasks_status = monitor_task->get_threads_status();
  for (auto status : tasks_status) {
    auto status_json = JsonObject();
    status_json.set("cpu", status.cpu_usage);
    status_json.set("minFreeStack", status.min_free_stack);
    status_json.set("currentPriority", status.current_priority);
    status_json.set("basePriority", status.base_priority);
    status_json.set("state", status.get_state_view());
    if (status.stack_size) {
      status_json.set("stackSize", status.stack_size);
    }
    task_status_json.set(status.name, &status_json);
  }
  return task_status_json;
}

// void WebSocketTask::fill_ota_progress_json(JsonObject* ota_json) {
//   ota_json->set("total", ota_total);
//   ota_json->set("progress", ota_progress);
// }

esp_err_t WebSocketTask::send_to_connection(int fd, std::string& data) {
  httpd_ws_frame_t ws_packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())),
      .len = data.size(),
  };
  return httpd_ws_send_data(http_server.server_instance, fd, &ws_packet);
}

esp_err_t WebSocketTask::send_to_connection(int fd, JsonObject* json) {
  auto json_str = json->stringify();
  return send_to_connection(fd, json_str);
}

void WebSocketTask::send_to_connections(WebSocketConnectionType connection_type, JsonObject* json) {
  auto connections = get_connections(connection_type);
  for (auto connection : connections) {
    auto status = send_to_connection(connection.fd, json);
    handle_ws_transmit_status(connection, status);
  }
}

esp_err_t WebSocketTask::send_to_legacy_connection(int fd) {
  LegacyDataPacket packet;
  auto left_motor_position = motor_task->get_position(0);
  auto right_motor_position = motor_task->get_position(1);
  packet.status = 1;
  packet.action = 2;
  packet.rp = right_motor_position;
  packet.lp = left_motor_position;
  packet.rt = 0;
  packet.lt = 0;
  packet.r_tmp = 0;
  packet.l_tmp = 0;
  packet.b_tmp = 0;
  httpd_ws_frame_t ws_packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_BINARY,
      .payload = reinterpret_cast<uint8_t*>(&packet),
      .len = sizeof(packet),
  };
  return httpd_ws_send_data(http_server.server_instance, fd, &ws_packet);
}

bool WebSocketTask::has_connection(WebSocketConnectionType connection_type) {
  connections = get_connections();
  for (auto connection : connections) {
    if (connection.stream == connection_type) {
      return true;
    }
  }
  return false;
}

void WebSocketTask::handle_ws_transmit_status(WebSocketConnection connection, esp_err_t status) {
  if (status == ESP_FAIL) {
    remove_connection(connection.fd);
  } else if (status == ESP_ERR_NO_MEM) {
    LOGE("failed to allocated memory while sending to connection (fd: %d, type: %s)",
         connection.fd,
         get_ws_stream_string(connection.stream));
  } else if (status != ESP_OK) {
    LOGE("unhandled error occurred while sending to connection (fd: %d, type: %s). %s",
         connection.fd,
         get_ws_stream_string(connection.stream),
         esp_err_to_name(status));
  }
}

void WebSocketTask::send_to_sys_log_connections(const char* data) {
  auto sys_log_connections = get_connections(WebSocketConnectionType::SYS_LOG);
  if (sys_log_connections.empty()) {
    return;
  }
  auto data_str = std::string(data);
  for (auto connection : sys_log_connections) {
    auto status = send_to_connection(connection.fd, data_str);
    handle_ws_transmit_status(connection, status);
  }
}

void WebSocketTask::main() {
  JsonObject task_status_json;
  JsonObject imu_data_json;
  JsonObject motor_data_json;
  JsonObject esp_heap;
  JsonObject ota_progress;

  while (true) {
    Sync::wait_for_notification();
    LOGI("woke up");
    while (true) {
      connection_mutex.take();
      auto no_connections = connections.empty();
      connection_mutex.give();
      if (no_connections) {
        LOGI("no connections available. going to sleep");
        break;
      }
      task_status_json = JsonObject();
      imu_data_json = JsonObject();
      motor_data_json = JsonObject();
      esp_heap = JsonObject();
      ota_progress = JsonObject();
      auto connections = get_connections();
      esp_err_t ret = ESP_FAIL;
      for (auto connection : connections) {
        switch (connection.stream) {
          case WebSocketConnectionType::IMU: {
            if (imu_data_json.is_empty()) {
              fill_imu_data_json(&imu_data_json);
            }
            ret = send_to_connection(connection.fd, &imu_data_json);
            break;
          }
          case WebSocketConnectionType::MOTOR: {
            if (motor_data_json.is_empty()) {
              fill_motor_data_json(&motor_data_json);
            }
            ret = send_to_connection(connection.fd, &motor_data_json);
            break;
          }
          default:
            ret = ESP_OK;
            break;
        }
        handle_ws_transmit_status(connection, ret);
      }
      while (true) {
        auto sys_log_result = sys_log_queue.receive(0);
        if (!sys_log_result.has_value()) {
          break;
        }
        auto sys_log = sys_log_result.value();
        send_to_sys_log_connections(sys_log);
        delete sys_log;
      };
      Sync::sleep(10);
    }
  }
}

void WebSocketTask::add_connection(int fd, WebSocketConnectionType stream) {
  LOGI("connection opened %d", fd);
  connection_mutex.take();
  for (auto& connection : connections) {
    if (connection.fd == fd) {
      connection.stream = stream;
      goto cleanup;
    }
  }

  connections.push_back({
      .fd = fd,
      .stream = stream,
  });

  if (connections.size() == 1) {
    notify();
  }

cleanup:
  connection_mutex.give();
}

void WebSocketTask::remove_connection(int fd) {
  connection_mutex.take();
  int connection_index_to_remove = -1;
  for (int i = 0; i < connections.size(); i++) {
    auto connection = connections[i];
    if (connection.fd == fd) {
      connection_index_to_remove = i;
      break;
    }
  }
  if (connection_index_to_remove != -1) {
    connections.erase(connections.begin() + connection_index_to_remove);
  }
  connection_mutex.give();
  LOGI("connection closed %d", fd);
}

std::vector<WebSocketConnection> WebSocketTask::get_connections() {
  connection_mutex.take();
  auto connections_copy = this->connections;
  connection_mutex.give();
  return connections_copy;
}

std::vector<WebSocketConnection> WebSocketTask::get_connections(
    WebSocketConnectionType connection_type) {
  std::vector<WebSocketConnection> filtered_connections;
  connection_mutex.take();
  for (auto connection : this->connections) {
    if (connection.stream == connection_type) {
      filtered_connections.push_back(connection);
    }
  }
  connection_mutex.give();
  return filtered_connections;
}

void WebSocketTask::send_to_task_connections(std::vector<ThreadStatus>& thread_status) {
  JsonObject json = fill_task_status_json(thread_status);
  send_to_connections(WebSocketConnectionType::TASK, &json);
}

JsonObject WebSocketTask::fill_motor_status_json(int motor_index,
                                                 int motor_id,
                                                 MotorStatus status) {
  JsonObject json;
  json.set("motorId", motor_id);
  json.set("motorIndex", motor_index);
  json.set("status", get_motor_status_string(status));
  return json;
}

void WebSocketTask::send_to_motor_status_connections(int motor_index,
                                                     int motor_id,
                                                     MotorStatus status) {
  auto json = fill_motor_status_json(motor_index, motor_id, status);
  send_to_connections(WebSocketConnectionType::MOTOR_STATUS, &json);
}