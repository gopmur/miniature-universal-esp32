#pragma once

#include <string>
#include <utility>
#include <vector>
#include "jayson.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread.hpp"
#include "system_logger.hpp"

enum class WsStream {
  TASK,
  IMU,
  MOTOR,
  OTA,
  SYS_LOG,
  LEGACY,
};

struct WebSocketConnections {
  int fd;
  WsStream stream;
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
  std::vector<WebSocketConnections> connections;
  Mutex connection_mutex;
  void fill_task_status_json(JsonObject* task_status_json);
  void fill_ota_progress_json(JsonObject* ota_json);
  void fill_imu_data_json(JsonObject* imu_data_json);
  void fill_motor_data_json(JsonObject* motor_data_json);
  void add_time_stamp(JsonObject* json);
  esp_err_t send_to_legacy_connection(int fd);
  esp_err_t send_to_connection(int fd, std::string& data);
  esp_err_t send_to_connection(int fd, JsonObject* json);

  public:
  void main();
  WebSocketTask();
  std::vector<WebSocketConnections> get_connections();
  void remove_connection(int fd);
  void add_connection(int fd, WsStream stream);
  Queue<std::pair<int, std::string*>, 8> sys_log_queue;
};