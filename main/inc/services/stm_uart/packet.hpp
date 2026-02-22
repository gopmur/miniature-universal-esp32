#pragma once

#include <array>
#include <cstdint>
#include "config.hpp"

enum class UartPacketType : uint8_t {
  START,
  STOP,
  SET_LEFT_TORQUE,
  SET_RIGHT_TORQUE,
  SET_MODE,
  GET_RUNNING,
  GET_RIGHT_MANUAL_TORQUE,
  GET_LEFT_MANUAL_TORQUE,
  GET_MODE
};

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};

const char* get_contorl_mode_str(ControlMode mode);

union UartPayload {
  float torque;
  ControlMode control_mode;
};

class UartPacket {
  public:
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_start_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_stop_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_set_left_torque_packet(float torque);
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_set_right_torque_packet(float torque);
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_set_mode_packet(ControlMode mode);
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_get_running_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_get_right_manual_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_get_left_manual_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_get_mode_packet();
};