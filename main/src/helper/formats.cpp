#include "helper/formats.hpp"
#include <sys/dirent.h>
#include "tasks/ws.hpp"

char get_hex_char(uint8_t byte) {
  byte &= 0xf;
  if (byte >= 0xa) {
    return 'a' + (byte - 0xa);
  } else {
    return '0' + byte;
  }
}

StaticString<2> get_hex(uint8_t byte) {
  StaticString<2> hex;
  auto h = byte >> 4;
  auto l = byte & 0xf;
  hex.push(get_hex_char(h));
  hex.push(get_hex_char(l));
  return hex;
}

StaticString<17> get_bssid_string(uint8_t bssid[6]) {
  StaticString<17> out;
  for (int i = 0; i < 6; i++) {
    uint8_t byte = bssid[i];
    auto byte_str = get_hex(byte);
    out.push(byte_str[0]);
    out.push(byte_str[1]);
    if (i != 5) {
      out.push(':');
    }
  }
  return out;
}

const char* get_entry_type_string(uint8_t entry_type) {
  switch (entry_type) {
    case DT_REG:
      return "file";
    case DT_DIR:
      return "directory";
  }
  return "unsupported";
}

const char* get_ws_stream_string(WebSocketConnectionType stream) {
  switch (stream) {
    case WebSocketConnectionType::TASK:
      return "tasks";
    case WebSocketConnectionType::IMU:
      return "imu";
    case WebSocketConnectionType::MOTOR:
      return "motor";
    case WebSocketConnectionType::OTA:
      return "ota";
    case WebSocketConnectionType::SYS_LOG:
      return "sys_log";
    case WebSocketConnectionType::LEGACY:
      return "legacy";
    case WebSocketConnectionType::MOTOR_STATUS:
      return "motor_status";
  }
  return "unknown";
}

const char* get_control_mode_string(ControlMode control_mode) {
  switch (control_mode) {
    case ControlMode::HEXA_MANUAL:
      return "manual";
    case ControlMode::HEXA_AUTOMATIC:
      return "automatic";
    case ControlMode::HEXA_SEMIAUTOMATIC:
      return "semiautomatic";
    case ControlMode::HEXA_SMART:
      return "smart";
  }
  return "unkown";
}

const char* get_motor_status_string(MotorStatus status) {
  switch (status) {
    case MotorStatus::OK:
      return "ok";
    case MotorStatus::TIMEDOUT:
      return "timeout";
    case MotorStatus::UNINITIALIZED:
      return "uninitialized";
    case MotorStatus::CAN_ERROR:
      return "can_error";
    case MotorStatus::INVALID:
      return "invalid";
  }
  return "unkown";
}