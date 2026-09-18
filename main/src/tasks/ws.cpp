#include "tasks/ws.hpp"
#include <optional>
#include <vector>
#include "config.hpp"

#include "custom_drivers/motor.hpp"
#include "esp_heap_caps.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "helper/json.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/sync.hpp"
#include "sdkconfig.h"
#include "tasks/http.hpp"
#include "tasks/imu.hpp"

extern ImuTask* imu_task;
extern HttpService* http_service;
extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;

WebSocketTask::WebSocketTask() : connection_mutex(true) {};

void WebSocketTask::fill_esp_task_data_json(JsonObject* esp_cpu_usage_json,
                                            JsonObject* esp_heap_json) {
  UBaseType_t taskCount = uxTaskGetNumberOfTasks();
  std::vector<TaskStatus_t> statusArray(taskCount);
  uint64_t totalRunTime = 0;

  taskCount = uxTaskGetSystemState(statusArray.data(), taskCount, &totalRunTime);

  if (totalRunTime == 0) {
    totalRunTime = 1;
  }

  for (UBaseType_t i = 0; i < taskCount; i++) {
    uint32_t taskTime = statusArray[i].ulRunTimeCounter;
    float cpuPercent = ((float)taskTime / (float)totalRunTime) * 100.0f;

    auto name = statusArray[i].pcTaskName;
    esp_cpu_usage_json->add_object(name);
    auto service_object = std::get<JsonObject>(esp_cpu_usage_json->get_object(name));
    service_object.set("cpuUsage", cpuPercent);
    service_object.set("minFreeStack", 0);
  }

  esp_heap_json->set("free", esp_get_free_heap_size());
  esp_heap_json->set("minFree", esp_get_minimum_free_heap_size());
  esp_heap_json->set("totalSize", heap_caps_get_total_size(MALLOC_CAP_DEFAULT));
  esp_heap_json->set("largestBlock", heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
}

void WebSocketTask::fill_json_with_packet_data(JsonObject* stm_cpu_usage_json,
                                               JsonObject* imu_data_json,
                                               JsonObject* motor_data_json,
                                               JsonObject* stm_heap) {
  // if (this->stream_is_enabled(WsStream::STM_TASK_DATA)) {
  //   switch (packet.address) {
  //     case SspAddress::LED_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("led");
  //       auto child_object_result = stm_cpu_usage_json->get_object("led");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::IMU_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("imu");
  //       auto child_object_result = stm_cpu_usage_json->get_object("imu");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }

  //     case SspAddress::MOTOR_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("motor");
  //       auto child_object_result = stm_cpu_usage_json->get_object("motor");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::SD_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("sd");
  //       auto child_object_result = stm_cpu_usage_json->get_object("sd");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::CAN_RECV_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("canRecv");
  //       auto child_object_result = stm_cpu_usage_json->get_object("canRecv");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::ESP_UART_RX_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("uartRx");
  //       auto child_object_result = stm_cpu_usage_json->get_object("uartRx");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::ESP_UART_TX_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("uartTx");
  //       auto child_object_result = stm_cpu_usage_json->get_object("uartTx");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::MONITOR_SERVICE_CPU_USAGE: {
  //       stm_cpu_usage_json->add_object("monitor");
  //       auto child_object_result = stm_cpu_usage_json->get_object("monitor");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("cpuUsage", packet.get_float());
  //       break;
  //     }
  //     case SspAddress::LED_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("led");
  //       auto child_object_result = stm_cpu_usage_json->get_object("led");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::IMU_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("imu");
  //       auto child_object_result = stm_cpu_usage_json->get_object("imu");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }

  //     case SspAddress::MOTOR_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("motor");
  //       auto child_object_result = stm_cpu_usage_json->get_object("motor");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::SD_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("sd");
  //       auto child_object_result = stm_cpu_usage_json->get_object("sd");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::CAN_RECV_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("canRecv");
  //       auto child_object_result = stm_cpu_usage_json->get_object("canRecv");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::ESP_UART_RX_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("uartRx");
  //       auto child_object_result = stm_cpu_usage_json->get_object("uartRx");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::ESP_UART_TX_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("uartTx");
  //       auto child_object_result = stm_cpu_usage_json->get_object("uartTx");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     case SspAddress::MONITOR_SERVICE_MIN_STACK_FREE: {
  //       stm_cpu_usage_json->add_object("monitor");
  //       auto child_object_result = stm_cpu_usage_json->get_object("monitor");
  //       auto child_object = std::get<JsonObject>(child_object_result);
  //       child_object.set("minFreeStack", packet.get_uint32());
  //       break;
  //     }
  //     // case SspAddress::HEAP_FREE:
  //     //   stm_heap.set_
  //     default:
  //       break;
  //   }
  // }

  // if (this->stream_is_enabled(WsStream::MOTOR_DATA)) {
  //   switch (packet.address) {
  //     case SspAddress::MOTOR_POS_LEFT:
  //       motor_data_json->set("leftPosition", packet.get_float());
  //       break;
  //     case SspAddress::MOTOR_POS_RIGHT:
  //       motor_data_json->set("rightPosition", packet.get_float());
  //       break;
  //     default:
  //       break;
  //   }
  // }
}

void WebSocketTask::fill_imu_data_json(JsonObject* imu_data_json) {
  imu_data_json->set("x", imu_task->data.gyro.x);
  imu_data_json->set("y", imu_task->data.gyro.y);
  imu_data_json->set("z", imu_task->data.gyro.z);
}

void WebSocketTask::fill_motor_data_json(JsonObject* motor_data_json) {
  motor_data_json->set("leftPosition", left_motor->get_position());
  motor_data_json->set("rightPosition", right_motor->get_position());
}

// void WebSocketTask::fill_ota_progress_json(JsonObject* ota_json) {
//   ota_json->set("total", ota_total);
//   ota_json->set("progress", ota_progress);
// }

void WebSocketTask::fill_root_json(JsonObject* json,
                                   JsonObject* stm_cpu_usage_json,
                                   JsonObject* esp_cpu_usage_json,
                                   JsonObject* imu_data_json,
                                   JsonObject* motor_data_json,
                                   JsonObject* esp_heap,
                                   JsonObject* ota_progress) {
  // if (this->stream_is_enabled(WsStream::STM_TASK_DATA))
  //   json->set("stmCpuUsage", stm_cpu_usage_json);
  // if (this->stream_is_enabled(WsStream::ESP_TASK_DATA))
  //   json->set("espCpuUsage", esp_cpu_usage_json);
  // if (this->stream_is_enabled(WsStream::ESP_TASK_DATA))
  //   json->set("espHeap", esp_heap);
  // if (this->stream_is_enabled(WsStream::IMU_DATA))
  //   json->set("imu", imu_data_json);
  // if (this->stream_is_enabled(WsStream::MOTOR_DATA))
  //   json->set("motor", motor_data_json);
  // if (this->stream_is_enabled(WsStream::OTA_PROGRESS))
  //   json->set("ota_progress", ota_progress);
}

esp_err_t WebSocketTask::send_to_connection(int fd, const char* data) {
  httpd_ws_frame_t ws_packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = (uint8_t*)data,
      .len = strlen(data),
  };
  return httpd_ws_send_data(http_service->server_instance, fd, &ws_packet);
}

void WebSocketTask::main() {
  JsonObject esp_cpu_usage_json;
  JsonObject imu_data_json;
  JsonObject motor_data_json;
  JsonObject esp_heap;
  JsonObject ota_progress;

  while (true) {
    Sync::wait_for_notification();
    ESP_LOGI(tag.c_str(), "woke up");
    while (true) {
      connection_mutex.take();
      auto no_connections = connections.empty();
      connection_mutex.give();
      if (no_connections) {
        ESP_LOGI(tag.c_str(), "no connections available. going to sleep");
        break;
      }

      esp_cpu_usage_json = JsonObject();
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
            if (imu_data_json.is_empty())
              fill_imu_data_json(&imu_data_json);
            auto imu_str = imu_data_json.stringify();
            ret = send_to_connection(connection.fd, imu_str.c_str());
            break;
          }
          default:
            ESP_LOGE(tag.c_str(),
                     "unhandled outgoing stream %d",
                     static_cast<uint32_t>(connection.stream));
            break;
        }
        if (ret != ESP_OK) {
          stop_sending(connection.fd);
        }
      }
      Sync::sleep(30);
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