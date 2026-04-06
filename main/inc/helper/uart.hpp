#pragma once

#include <initializer_list>
#include "config.hpp"
#include "driver/uart.h"
#include "services/stm_uart/ssp.hpp"

void start_streams(std::initializer_list<SspAddress> addresses);
void stop_streams(std::initializer_list<SspAddress> addresses);
void read_addresses(std::initializer_list<SspAddress> addresses);
void send_command(SspCommand command);

template <typename T>
void write_address(SspAddress address, T value) {
  auto packet = SspPacket::make_write_packet(address, value).get_raw_packet();
  uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
}