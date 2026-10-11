#pragma once

#include <string>
#include <vector>
#include "custom_drivers/motor.hpp"
#include "http.hpp"
#include "jayson.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread.hpp"
#include "system_logger.hpp"

enum class WebSocketConnectionType {
  TASK,
  IMU,
  MOTOR,
  OTA,
  SYS_LOG,
  LEGACY,
  HEAP,
  MOTOR_STATUS,
};

struct WebSocketConnection {
  int fd;
  WebSocketConnectionType stream;
};

struct __attribute__((packed)) LegacyDataPacket {
  uint8_t status;
  uint8_t action;
  float rp;
  float lp;
  float rt;
  float lt;
  float r_tmp;
  float l_tmp;
  float b_tmp;
};

class WebSocketTask : public Thread {
  MAKE_LOGGABLE("websocket_task");

  private:
  std::vector<WebSocketConnection> connections;
  Mutex connection_mutex;
  JsonObject fill_task_status_json(std::vector<ThreadStatus>& thread_status);
  JsonObject fill_motor_status_json(int motor_index, int motor_id, MotorStatus status);
  void fill_ota_progress_json(JsonObject* ota_json);
  void fill_imu_data_json(JsonObject* imu_data_json);
  void fill_motor_data_json(JsonObject* motor_data_json);
  void fill_heap_data_json(JsonObject* heap_data_json);
  void add_time_stamp(JsonObject* json);
  esp_err_t send_to_legacy_connection(int fd);
  esp_err_t send_to_connection(int fd, std::string& data);
  esp_err_t send_to_connection_async();
  esp_err_t send_to_connection(int fd, JsonObject* json);
  void send_to_connections(WebSocketConnectionType connection_type, JsonObject* json);
  std::vector<WebSocketConnection> get_connections();
  std::vector<WebSocketConnection> get_connections(WebSocketConnectionType connection_type);
  void handle_ws_transmit_status(WebSocketConnection connection, esp_err_t status);
  void send_to_sys_log_connections(const char* data);
  void send_initial_motor_status(int fd);

  public:
  bool has_connection(WebSocketConnectionType connection_type);
  void send_to_task_connections(std::vector<ThreadStatus>& thread_status);
  void send_to_motor_status_connections(int motor_index, int motor_id, MotorStatus status);
  void main();
  WebSocketTask();
  void remove_connection(int fd);
  void add_connection(int fd, WebSocketConnectionType stream);
  Queue<char*, 8> sys_log_queue;
};