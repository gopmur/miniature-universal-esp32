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
  UartPacketType type;
  UartPayload payload;

  std::array<uint8_t, config::stm_uart::packet_length> get_raw();
};