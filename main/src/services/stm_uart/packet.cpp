#include <cstdint>
#include "config.hpp"
#include "helper.hpp"
#include "services/stm_uart_tx.hpp"

const char* get_contorl_mode_str(ControlMode mode) {
  switch (mode) {
    case ControlMode::MANUAL:
      return "manual";
    case ControlMode::AUTO:
      return "automatic";
    case ControlMode::SEMI_AUTO:
      return "semi-automatic";
    case ControlMode::SMART:
      return "smart";
  }
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_start_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::START);
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_stop_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::STOP);
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_set_left_torque_packet(float torque) {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::SET_LEFT_TORQUE);
  for (int i = 0; i < 4; i++) {
    packet[i + 1] = get_byte(torque, i);
  }
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_set_right_torque_packet(float torque) {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::SET_RIGHT_TORQUE);
  for (int i = 0; i < 4; i++) {
    packet[i + 1] = get_byte(torque, i);
  }
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_set_mode_packet(ControlMode mode) {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::SET_MODE);
  packet[1] = static_cast<uint8_t>(mode);
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_get_running_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::GET_RUNNING);
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_get_right_manual_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::GET_RIGHT_MANUAL_TORQUE);
  return packet;
}

std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_get_left_manual_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::GET_LEFT_MANUAL_TORQUE);
  return packet;
}
std::array<uint8_t, config::stm_uart::packet_length>
UartPacket::make_get_mode_packet() {
  std::array<uint8_t, config::stm_uart::packet_length> packet;
  packet[0] = static_cast<uint8_t>(UartPacketType::GET_MODE);
  return packet;
}