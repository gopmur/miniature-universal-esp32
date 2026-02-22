#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include "config.hpp"

class Lappl {
  private:
  static int i;
  static std::array<uint8_t, config::stm_uart::packet_length> decoded_packet;

  public:
  static std::array<uint8_t, config::stm_uart::packet_length + 1> encode(
      std::array<uint8_t, config::stm_uart::packet_length> packet);
  static std::optional<std::array<uint8_t, config::stm_uart::packet_length>>
  read(uint8_t lappl_byte);
};