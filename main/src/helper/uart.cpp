#include "helper/uart.hpp"

void start_streams(std::initializer_list<LapplAddress> addresses) {
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

void stop_streams(std::initializer_list<LapplAddress> addresses) {
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

void read_addresses(std::initializer_list<LapplAddress> addresses) {
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

void send_command(LapplCommand command) {
  auto packet = LapplPacket::make_command_packet(command).get_raw_packet();
  uart_write_bytes(config::stm_uart::port, packet.data(), packet.size());
}