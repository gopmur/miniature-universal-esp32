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
#include "ipc/mutex.hpp"
#include "jaythread/sync.hpp"
#include "sdkconfig.h"
#include "tasks/http.hpp"
#include "tasks/imu.hpp"

extern ImuTask* imu_task;
extern HttpService* http_service;
extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;

WebSocketTask::WebSocketTask() : connection_mutex(true) {
  connection_age.fill(-1);
  connection_fds.fill(-1);
  connection_count = 0;
  stream_enabled.fill(false);
};

bool WebSocketTask::stream_is_enabled(WsStream stream) {
  return this->stream_enabled[static_cast<size_t>(stream)];
}

void WebSocketTask::enable_stream(WsStream stream) {
  if (this->stream_is_enabled(stream)) {
    return;
  }
  enabled_stream_count++;
  this->stream_enabled[static_cast<size_t>(stream)] = true;
  if (enabled_stream_count == 1) {
    this->resume();
  }
}
void WebSocketTask::disable_stream(WsStream stream) {
  if (!this->stream_is_enabled(stream)) {
    return;
  }
  enabled_stream_count--;
  this->stream_enabled[static_cast<size_t>(stream)] = false;
  if (enabled_stream_count == 0) {
    this->suspend();
  }
}

bool WebSocketTask::has_connections() {
  return connection_count > 0;
}

void WebSocketTask::fill_esp_task_data_json(JsonObject* esp_cpu_usage_json,
                                            JsonObject* esp_heap_json) {
  if (!this->stream_is_enabled(WsStream::ESP_TASK_DATA)) {
    return;
  }

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
  if (this->stream_is_enabled(WsStream::IMU_DATA)) {
    imu_data_json->set("x", imu_task->data.gyro.x);
    imu_data_json->set("y", imu_task->data.gyro.y);
    imu_data_json->set("z", imu_task->data.gyro.z);
  }
}

void WebSocketTask::fill_motor_data_json(JsonObject* motor_data_json) {
  if (this->stream_is_enabled(WsStream::MOTOR_DATA)) {
    motor_data_json->set("leftPosition", left_motor->get_position());
    motor_data_json->set("rightPosition", right_motor->get_position());
  }
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
  if (this->stream_is_enabled(WsStream::STM_TASK_DATA))
    json->set("stmCpuUsage", stm_cpu_usage_json);
  if (this->stream_is_enabled(WsStream::ESP_TASK_DATA))
    json->set("espCpuUsage", esp_cpu_usage_json);
  if (this->stream_is_enabled(WsStream::ESP_TASK_DATA))
    json->set("espHeap", esp_heap);
  if (this->stream_is_enabled(WsStream::IMU_DATA))
    json->set("imu", imu_data_json);
  if (this->stream_is_enabled(WsStream::MOTOR_DATA))
    json->set("motor", motor_data_json);
  if (this->stream_is_enabled(WsStream::OTA_PROGRESS))
    json->set("ota_progress", ota_progress);
}

void WebSocketTask::send_to_connections(const char* data) {
  connection_mutex.take();
  httpd_ws_frame_t ws_packet = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = (uint8_t*)data,
      .len = strlen(data),
  };
  for (int i = 0; i < this->connection_count; i++) {
    int fd = this->connection_fds[i];
    esp_err_t ret = httpd_ws_send_data(http_service->server_instance, fd, &ws_packet);

    if (ret != ESP_OK) {
      this->stop_sending(fd);
      if (this->connection_count == 0) {
        connection_mutex.give();
        this->suspend();
        return;
      }
    }
  }
  connection_mutex.give();
}

void WebSocketTask::main() {
  JsonObject json;
  JsonObject esp_cpu_usage_json;
  JsonObject stm_cpu_usage_json;
  JsonObject imu_data_json;
  JsonObject motor_data_json;
  JsonObject esp_heap;
  JsonObject stm_heap;
  JsonObject ota_progress;

  auto json_str = json.stringify();
  while (true) {
    this->suspend();
    while (true) {
      uint64_t microseconds = esp_timer_get_time();
      if (!this->has_connections()) {
        break;
      }
      fill_imu_data_json(&imu_data_json);
      fill_motor_data_json(&motor_data_json);
      fill_esp_task_data_json(&esp_cpu_usage_json, &esp_heap);
      fill_root_json(&json,
                     &stm_cpu_usage_json,
                     &esp_cpu_usage_json,
                     &imu_data_json,
                     &motor_data_json,
                     &esp_heap,
                     &ota_progress);
      // this->fill_ota_progress_json(&ota_progress);

      if (!json.is_empty()) {
        json.set("microseconds", microseconds);
        this->send_to_connections(json.stringify().c_str());
        json = JsonObject();
        esp_cpu_usage_json = JsonObject();
        stm_cpu_usage_json = JsonObject();
        imu_data_json = JsonObject();
      }
      Sync::sleep(30);
    }
  }
}

void WebSocketTask::start_sending(int fd) {
  connection_mutex.take();
  for (int i = 0; i < this->connection_count; i++) {
    if (this->connection_fds[i] == fd) {
      goto cleanup;
    }
  }
  if (connection_count < CONFIG_HEXA_WS_MAX_CONNECTIONS) {
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

cleanup:
  connection_mutex.give();
}

void WebSocketTask::stop_sending(int fd) {
  connection_mutex.take();
  int connection_to_remove = -1;
  int connection_to_remove_age = 0;
  if (this->connection_count == 0) {
    goto cleanup;
  }
  for (int i = 0; i < this->connection_count; i++) {
    if (this->connection_fds[i] == fd) {
      connection_to_remove = i;
      connection_to_remove_age = this->connection_age[i];
      break;
    }
  }
  if (connection_to_remove == -1) {
    goto cleanup;
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
cleanup:
  connection_mutex.give();
}