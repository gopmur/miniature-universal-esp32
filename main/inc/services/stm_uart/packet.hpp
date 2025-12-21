#include <array>
#include <cstdint>
#include "config.hpp"

enum class UartPacketType : uint8_t {
  START,
  STOP,
  SET_LEFT_TORQUE,
  SET_RIGHT_TORQUE,
  SET_MODE,
};

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};

union UartPayload {
  float torque;
  ControlMode control_mode;
};

struct UartPacket {
  UartPacketType header;
  std::array<uint8_t, config::stm_uart::packet_length - 1> payload;

  static std::array<uint8_t, config::stm_uart::packet_length>
  make_start_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_stop_packet();
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_set_left_torque_packet(float torque);
  static std::array<uint8_t, config::stm_uart::packet_length>
  make_set_right_torque_packet(float torque);
  static std::array<uint8_t, config::stm_uart::packet_length> make_set_mode_packet(
      ControlMode mode);
};