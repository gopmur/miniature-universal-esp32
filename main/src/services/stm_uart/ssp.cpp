#include <cstdint>
#include <optional>
#include <services/stm_uart/ssp.hpp>
#include "esp_log.h"
#include "helper.hpp"

int Ssp::i;
SspPacket Ssp::packet;

float SspPacket::get_float() {
  return f_concat(this->data[0], this->data[1], this->data[2], this->data[3]);
}
uint32_t SspPacket::get_uint32() {
  return u32_concat(this->data[0], this->data[1], this->data[2], this->data[3]);
}

uint16_t SspPacket::get_uint16() {
  return u16_concat(this->data[0], this->data[1]);
}

uint8_t SspPacket::get_uint8() {
  return this->data[0];
}

bool SspPacket::get_bool() {
  return this->data[0];
}

SspPacket SspPacket::make_read_packet(SspAddress address) {
  SspPacket packet;
  packet.header.b.type = SspType::READ;
  packet.header.b.resp = 0;
  packet.address = address;
  for (int i = 0; i < 4; i++) {
    packet.data[i] = 0;
  }
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_start_stream_packet(SspAddress address) {
  SspPacket packet;
  packet.header.b.type = SspType::START_STREAM;
  packet.header.b.resp = 0;
  packet.address = address;
  for (int i = 0; i < 4; i++) {
    packet.data[i] = 0;
  }
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_stop_stream_packet(SspAddress address) {
  SspPacket packet;
  packet.header.b.type = SspType::STOP_STREAM;
  packet.header.b.resp = 0;
  packet.address = address;
  for (int i = 0; i < 4; i++) {
    packet.data[i] = 0;
  }
  packet.pack();
  return packet;
}

void SspPacket::pack() {
  this->generate_check_sum();
  this->generate_sign_byte();
  this->remove_sign_bits();
}

SspPacket SspPacket::make_eoc_packet() {
  SspPacket packet;
  packet.header.b.type = SspType::EOC;
  packet.header.b.resp = 1;
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_command_packet(SspCommand command) {
  SspPacket packet;
  packet.header.b.type = SspType::COMMAND;
  packet.header.b.resp = 0;
  packet.data[0] = static_cast<uint8_t>(command);
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_read_response_packet(SspAddress address,
                                                 std::array<uint8_t, 4> data) {
  SspPacket packet;
  packet.header.b.type = SspType::READ;
  packet.header.b.resp = 1;
  packet.address = address;
  for (int i = 0; i < 4; i++) {
    packet.data[i] = data[i];
  }
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_read_response_packet(SspAddress address,
                                                 bool data) {
  std::array<uint8_t, 4> raw_data = {data, 0, 0, 0};
  return make_read_response_packet(address, raw_data);
}

SspPacket SspPacket::make_read_response_packet(SspAddress address,
                                                 uint8_t data) {
  std::array<uint8_t, 4> raw_data = {data, 0, 0, 0};
  return make_read_response_packet(address, raw_data);
}

SspPacket SspPacket::make_read_response_packet(SspAddress address,
                                                 float data) {
  std::array<uint8_t, 4> raw_data = {get_byte(data, 0),
                                     get_byte(data, 1),
                                     get_byte(data, 2),
                                     get_byte(data, 3)};
  return make_read_response_packet(address, raw_data);
}

SspPacket SspPacket::make_write_packet(SspAddress address,
                                         std::array<uint8_t, 4> data) {
  SspPacket packet;
  packet.header.b.type = SspType::WRITE;
  packet.header.b.resp = 0;
  packet.address = address;
  for (int i = 0; i < 4; i++) {
    packet.data[i] = data[i];
  }
  packet.pack();
  return packet;
}

SspPacket SspPacket::make_write_packet(SspAddress address, bool data) {
  std::array<uint8_t, 4> raw_data = {data, 0, 0, 0};
  return make_write_packet(address, raw_data);
}

SspPacket SspPacket::make_write_packet(SspAddress address, uint8_t data) {
  std::array<uint8_t, 4> raw_data = {data, 0, 0, 0};
  return make_write_packet(address, raw_data);
}

SspPacket SspPacket::make_write_packet(SspAddress address, float data) {
  std::array<uint8_t, 4> raw_data = {get_byte(data, 0),
                                     get_byte(data, 1),
                                     get_byte(data, 2),
                                     get_byte(data, 3)};
  return make_write_packet(address, raw_data);
}

SspPacket SspPacket::make_write_packet(SspAddress address, uint16_t data) {
  std::array<uint8_t, 4> raw_data = {get_low(data), get_high(data), 0, 0};
  return make_write_packet(address, raw_data);
}
SspPacket SspPacket::make_write_packet(SspAddress address, uint32_t data) {
  std::array<uint8_t, 4> raw_data = {get_byte(data, 0),
                                     get_byte(data, 1),
                                     get_byte(data, 2),
                                     get_byte(data, 3)};
  return make_write_packet(address, raw_data);
}

SspPacket SspPacket::make_write_packet(SspAddress address, int8_t data) {
  return make_write_packet(address, static_cast<uint8_t>(data));
}
SspPacket SspPacket::make_write_packet(SspAddress address, int16_t data) {
  return make_write_packet(address, static_cast<uint16_t>(data));
}
SspPacket SspPacket::make_write_packet(SspAddress address, int32_t data) {
  return make_write_packet(address, static_cast<uint32_t>(data));
}

std::array<uint8_t, 8> SspPacket::get_raw_packet() {
  std::array<uint8_t, 8> raw_packet;
  raw_packet[0] = this->header.u8;
  raw_packet[1] = this->sign;
  raw_packet[2] = static_cast<uint8_t>(this->address);
  for (int i = 0; i < 4; i++) {
    raw_packet[i + 3] = this->data[i];
  }
  raw_packet[7] = this->check_sum;
  return raw_packet;
}

uint8_t SspPacket::calculate_check_sum() {
  uint8_t check_sum = 0;
  check_sum = this->header.u8;
  check_sum += static_cast<uint8_t>(this->address);
  for (int i = 0; i < 4; i++) {
    check_sum += this->data[i];
  }
  return check_sum;
}

void SspPacket::generate_check_sum() {
  this->check_sum = calculate_check_sum();
}

void SspPacket::generate_sign_byte() {
  this->sign = 0;

  int sign_bit = get_bit(this->header.u8, 7);
  this->sign |= sign_bit;
  this->sign <<= 1;

  sign_bit = get_bit(static_cast<uint8_t>(this->address), 7);
  this->sign |= sign_bit;
  this->sign <<= 1;

  for (int i = 0; i < 4; i++) {
    sign_bit = get_bit(this->data[i], 7);
    this->sign |= sign_bit;
    this->sign <<= 1;
  }

  sign_bit = get_bit(this->check_sum, 7);
  this->sign |= sign_bit;
}

void SspPacket::remove_sign_bits() {
  this->header.u8 = set_bit(header.u8, 7);
  this->address = static_cast<SspAddress>(
      unset_bit(static_cast<uint8_t>(this->address), 7));
  for (int i = 0; i < 4; i++) {
    this->data[i] = unset_bit(this->data[i], 7);
  }
  this->check_sum = unset_bit(this->check_sum, 7);
}

void SspPacket::recreate_sign_bits() {
  int sign_bit = get_bit(this->sign, 0) << 7;
  this->check_sum |= sign_bit;

  for (int i = 1; i < 5; i++) {
    sign_bit = get_bit(this->sign, i) << 7;
    this->data[4 - i] |= sign_bit;
  }

  sign_bit = get_bit(this->sign, 5) << 7;
  this->address =
      static_cast<SspAddress>(static_cast<uint8_t>(address) | sign_bit);

  sign_bit = get_bit(this->sign, 6) << 7;
  this->header.u8 &= sign_bit | (~0x80);
}

bool SspPacket::check_integrity() {
  uint8_t check_sum = this->calculate_check_sum();
  return check_sum == this->check_sum;
}

std::optional<SspPacket> Ssp::read_stream(uint8_t input) {
  if (get_bit(input, 7)) {
    i = 0;
    packet.header.u8 = input;
    i++;
  }

  else if (i == 1) {
    packet.sign = input;
    i++;
  }

  else if (i == 2) {
    packet.address = static_cast<SspAddress>(input);
    i++;
  }

  else if (i < 7) {
    packet.data[i - 3] = input;
    i++;
  }

  else if (i == 7) {
    packet.check_sum = input;
    packet.recreate_sign_bits();
    auto no_packet_errors = packet.check_integrity();
    if (no_packet_errors) {
      return packet;
    } else {
      i = 0;
    }
  }

  return std::nullopt;
}