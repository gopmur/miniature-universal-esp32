#pragma once

#include <initializer_list>
#include "config.hpp"
#include "driver/uart.h"
#include "services/stm_uart/lappl.hpp"

void start_streams(std::initializer_list<LapplAddress> addresses);
void stop_streams(std::initializer_list<LapplAddress> addresses);
void read_addresses(std::initializer_list<LapplAddress> addresses);
void send_command(LapplCommand command);

template <typename T>
void write_address(LapplAddress address, T value) {
  auto packet = LapplPacket::make_write_packet(address, value).get_raw_packet();
  uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
}