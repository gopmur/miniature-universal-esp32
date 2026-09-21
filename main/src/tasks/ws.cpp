#include "tasks/ws.hpp"
#include <vector>


#include "custom_drivers/motor.hpp"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "jayson.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/sync.hpp"
#include "http.hpp"
#include "tasks/imu.hpp"
#include "tasks/monitor.hpp"

extern ImuTask* imu_task;
extern HttpServer http_server;
extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;
extern MonitorTask* monitor_task;

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
  motor_data_json->set("leftPosition", left_motor->get_position());
  motor_data_json->set("rightPosition", right_motor->get_position());
}

void WebSocketTask::fill_task_status_json(JsonObject* task_status_json) {
  add_time_stamp(task_status_json);
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
    task_status_json->set(status.name, &status_json);
  }
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

      connection_mutex.take();
      auto connections_copy = this->connections;
      connection_mutex.give();
      esp_err_t ret;
      for (auto connection : connections_copy) {
        switch (connection.stream) {
          case WsStream::IMU_DATA: {
            if (imu_data_json.is_empty()) {
              fill_imu_data_json(&imu_data_json);
            }
            ret = send_to_connection(connection.fd, &imu_data_json);
            break;
          }
          case WsStream::MOTOR_DATA: {
            if (motor_data_json.is_empty()) {
              fill_motor_data_json(&motor_data_json);
            }
            ret = send_to_connection(connection.fd, &motor_data_json);
            break;
          }
          case WsStream::ESP_TASK_DATA: {
            if (task_status_json.is_empty()) {
              fill_task_status_json(&task_status_json);
            }
            ret = send_to_connection(connection.fd, &task_status_json);
            break;
          }
          default:
            LOGE("unhandled outgoing stream %d", static_cast<uint32_t>(connection.stream));
            break;
        }
        if (ret != ESP_OK) {
          stop_sending(connection.fd);
        }
      }
      Sync::sleep(10);
    }
  }
}

void WebSocketTask::start_sending(int fd, WsStream stream) {
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

void WebSocketTask::stop_sending(int fd) {
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
}