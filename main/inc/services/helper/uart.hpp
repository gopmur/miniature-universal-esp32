#pragma once

#include <initializer_list>
#include "config.hpp"
#include "driver/uart.h"
#include "services/stm_uart/lappl.hpp"

inline void start_streams(std::initializer_list<LapplAddress> addresses) {
  std::array<bool, static_cast<uint8_t>(LapplAddress::ADDRESS_COUNT)>
      address_sent;
  address_sent.fill(false);
  for (auto address : addresses) {
    if (address_sent[static_cast<uint8_t>(address)]) {
      continue;
    }
    address_sent[static_cast<uint8_t>(address)] = true;
    auto packet =
        LapplPacket::make_start_stream_packet(address).get_raw_packet();
    uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
  }
}

inline void stop_streams(std::initializer_list<LapplAddress> addresses) {
  std::array<bool, static_cast<uint8_t>(LapplAddress::ADDRESS_COUNT)>
      address_sent;
  address_sent.fill(false);
  for (auto address : addresses) {
    if (address_sent[static_cast<uint8_t>(address)]) {
      continue;
    }
    address_sent[static_cast<uint8_t>(address)] = true;
    auto packet =
        LapplPacket::make_stop_stream_packet(address).get_raw_packet();
    uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
  }
}

inline void read_addresses(std::initializer_list<LapplAddress> addresses) {
  std::array<bool, static_cast<uint8_t>(LapplAddress::ADDRESS_COUNT)>
      address_sent;
  address_sent.fill(false);
  for (auto address : addresses) {
    if (address_sent[static_cast<uint8_t>(address)]) {
      continue;
    }
    address_sent[static_cast<uint8_t>(address)] = true;
    auto packet = LapplPacket::make_read_packet(address).get_raw_packet();
    uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
  }
}

template <typename T>
inline void write_address(LapplAddress address, T value) {
  auto packet = LapplPacket::make_write_packet(address, value).get_raw_packet();
  uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
}