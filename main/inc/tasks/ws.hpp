#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include "config.hpp"
#include "helper/json.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread.hpp"
#include "sdkconfig.h"

enum class WsStream {
  ESP_TASK_DATA,
  STM_TASK_DATA,
  IMU_DATA,
  MOTOR_DATA,
  OTA_PROGRESS,

  COUNT,
};

struct WebSocketConnections {
  int fd;
  WsStream stream;
};

class WebSocketTask : public Thread {
  private:
  std::string tag = "ws task";
  std::vector<WebSocketConnections> connections;
  Mutex connection_mutex;
  void fill_esp_task_data_json(JsonObject* esp_cpu_usage_json, JsonObject* esp_heap_json);
  void fill_json_with_packet_data(JsonObject* stm_cpu_usage_json,
                                  JsonObject* imu_data_json,
                                  JsonObject* motor_data_json,
                                  JsonObject* stm_heap);
  void fill_root_json(JsonObject* json,
                      JsonObject* stm_cpu_usage_json,
                      JsonObject* esp_cpu_usage_json,
                      JsonObject* imu_data_json,
                      JsonObject* motor_data_json,
                      JsonObject* esp_heap,
                      JsonObject* ota_json);
  void fill_ota_progress_json(JsonObject* ota_json);
  void fill_imu_data_json(JsonObject* imu_data_json);
  void fill_motor_data_json(JsonObject* motor_data_json);
  void fill_task_status_json(JsonObject* task_status_json);
  void add_time_stamp(JsonObject* json);
  void stop_sending(int fd);
  esp_err_t send_to_connection(int fd, std::string& data);
  esp_err_t send_to_connection(int fd, JsonObject* json);

  public:
  void main();
  WebSocketTask();
  bool uart_streams_enabled();
  void start_sending(int fd, WsStream stream);
};