#pragma once

#include <initializer_list>
#include "config.hpp"
#include "driver/uart.h"
#include "services/stm_uart/rssp.hpp"

void start_streams(std::initializer_list<RsspAddress> addresses);
void stop_streams(std::initializer_list<RsspAddress> addresses);
void read_addresses(std::initializer_list<RsspAddress> addresses);
void send_command(RsspCommand command);

template <typename T>
void write_address(RsspAddress address, T value) {
  auto packet = RsspPacket::make_write_packet(address, value).get_raw_packet();
  uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
}