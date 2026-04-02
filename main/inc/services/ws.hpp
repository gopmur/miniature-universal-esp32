#pragma once

#include <cstddef>
#include "config.hpp"
#include "helper/json.hpp"
#include "ipc/queue.hpp"
#include "service.hpp"
#include "services/stm_uart/rssp.hpp"

enum class WsStream {
  ESP_TASK_DATA,
  STM_TASK_DATA,
  IMU_DATA,
  MOTOR_DATA,
  COUNT,
};

class WebSocketService : public Service<config::service::ws::stack_size> {
  private:
  std::array<bool, static_cast<size_t>(WsStream::COUNT)> stream_enabled;
  int enabled_stream_count = 0;
  std::array<int, config::service::ws::max_connection> connection_fds;
  std::array<int, config::service::ws::max_connection> connection_age;
  int connection_count;
  bool should_wait_for_eoc();
  void fill_esp_task_data_json(JsonObject* esp_cpu_usage_json, JsonObject* esp_heap_json);
  void fill_json_with_packet_data(RsspPacket packet,
                                  JsonObject* stm_cpu_usage_json,
                                  JsonObject* imu_data_json,
                                  JsonObject* motor_data_json,
                                  JsonObject* stm_heap);
  void fill_root_json(JsonObject* json,
                      JsonObject* stm_cpu_usage_json,
                      JsonObject* esp_cpu_usage_json,
                      JsonObject* imu_data_json,
                      JsonObject* motor_data_json,
                      JsonObject* esp_heap);
  void send_to_connections(const char* data);
  bool stream_is_enabled(WsStream stream);

  public:
  void main();
  Queue<RsspPacket, 32> queue;
  WebSocketService(int priority);
  bool has_connections();
  bool uart_streams_enabled();
  void start_sending(int fd);
  void stop_sending(int fd);

  void enable_stream(WsStream);
  void disable_stream(WsStream);
};