#pragma once

#include "config.hpp"
#include "helper/json.hpp"
#include "ipc/queue.hpp"
#include "service.hpp"
#include "services/stm_uart/rssp.hpp"

class WebSocketService
    : public AbstractService<WebSocketService, config::service::ws::stack_size> {
  private:
  bool esp_cpu_usage_enabled;
  bool stm_cpu_usage_enabled;
  bool imu_data_enabled;
  bool motor_data_enabled;
  std::array<int, config::service::ws::max_connection> connection_fds;
  std::array<int, config::service::ws::max_connection> connection_age;
  int connection_count;
  bool should_wait_for_eoc();
  void fill_esp_cpu_usage_json(JsonObject* esp_cpu_usage_json);
  void fill_json_with_packet_data(RsspPacket packet,
                                  JsonObject* stm_cpu_usage_json,
                                  JsonObject* imu_data_json,
                                  JsonObject* motor_data_json);
  void fill_root_json(JsonObject* json,
                      JsonObject* stm_cpu_usage_json,
                      JsonObject* esp_cpu_usage_json,
                      JsonObject* imu_data_json,
                      JsonObject* motor_data_json);
  void send_to_connections(const char* data);

  
  public:
  void main();
  Queue<RsspPacket, 32> queue;
  WebSocketService(int priority);
  bool has_connections();
  void start_sending(int fd);
  void stop_sending(int fd);

  void enable_esp_cpu_usage();
  void disable_esp_cpu_usage();
  void enable_stm_cpu_usage();
  void disable_stm_cpu_usage();
  void enable_imu_data();
  void disable_imu_data();
  void enable_motor_data();
  void disable_motor_data();
};