#include <cstdint>
#include "config.hpp"
#include "helper.hpp"
#include "services/stm_uart.hpp"

std::array<uint8_t, config::stm_uart::packet_length> UartPacket::get_raw() {
  std::array<uint8_t, config::stm_uart::packet_length> raw;
  raw[0] = static_cast<uint8_t>(this->type);
  switch (this->type) {
    case UartPacketType::SET_LEFT_TORQUE:
    case UartPacketType::SET_RIGHT_TORQUE:
      raw[1] = get_byte(this->payload.torque, 0);
      raw[2] = get_byte(this->payload.torque, 1);
      raw[3] = get_byte(this->payload.torque, 2);
      raw[4] = get_byte(this->payload.torque, 3);
      break;
    case UartPacketType::SET_MODE:
      raw[1] = static_cast<uint8_t>(this->payload.control_mode);
    default:
      break;
  }
  return raw;
}

