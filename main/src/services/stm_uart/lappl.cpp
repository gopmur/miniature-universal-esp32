#include <optional>
#include <services/stm_uart/lappl.hpp>
#include "config.hpp"

int Lappl::i = 0;
std::array<uint8_t, config::stm_uart::packet_length> Lappl::decoded_packet;

std::array<uint8_t, config::stm_uart::packet_length + 1> Lappl::encode(
    std::array<uint8_t, config::stm_uart::packet_length> packet) {
  std::array<uint8_t, config::stm_uart::packet_length + 1> encoded_packet;
  encoded_packet.fill(0);
  int sign_byte_index = config::stm_uart::packet_length;
  encoded_packet[0] = packet[0] | 0x80;
  encoded_packet[sign_byte_index] = 0;
  for (int i = 1; i < sign_byte_index; i++) {
    encoded_packet[sign_byte_index] <<= 1;
    encoded_packet[sign_byte_index] |= (packet[i] & 0x80) >> 7;
    encoded_packet[i] = packet[i] & (~0x80);
  }
  return encoded_packet;
}

std::optional<std::array<uint8_t, config::stm_uart::packet_length>> Lappl::read(
    uint8_t lappl_byte) {
  if (lappl_byte & 0x80) {
    i = 0;
    decoded_packet[i] = lappl_byte & (~0x80);
    i++;
  }

  else if (i < config::stm_uart::packet_length) {
    decoded_packet[i] = lappl_byte;
    i++;
  }

  else if (i == config::stm_uart::packet_length) {
    for (int j = 1; j < config::stm_uart::packet_length; j++) {
      int sign_bit = lappl_byte & 0x08;
      lappl_byte <<= 1;
      decoded_packet[j] |= sign_bit << 4;
    }
    return decoded_packet;
    i++;
  }

  return std::nullopt;
}
