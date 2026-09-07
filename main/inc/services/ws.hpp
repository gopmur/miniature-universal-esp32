#pragma once

#include <array>
#include <cstddef>
#include "config.hpp"
#include "helper/json.hpp"
#include "ipc/mutex.hpp"
#include "ipc/queue.hpp"
#include "jaythread/thread.hpp"

enum class WsStream {
  ESP_TASK_DATA,
  STM_TASK_DATA,
  IMU_DATA,
  MOTOR_DATA,
  OTA_PROGRESS,

  COUNT,
};

class WebSocketService : public Thread {
  private:
  std::array<bool, static_cast<size_t>(WsStream::COUNT)> stream_enabled;
  int enabled_stream_count = 0;
  std::array<int, config::service::ws::max_connection> connection_fds;
  std::array<int, config::service::ws::max_connection> connection_age;
  Mutex connection_mutex;
  int connection_count;
  bool should_wait_for_eoc();
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
  bool stream_is_enabled(WsStream stream);
  void fill_imu_data_json(JsonObject* imu_data_json);
  void stop_sending(int fd);

  public:
  void main();
  WebSocketService();
  void send_to_connections(const char* data);
  bool has_connections();
  bool uart_streams_enabled();
  void start_sending(int fd);

  void enable_stream(WsStream);
  void disable_stream(WsStream);
};