#include <cstdint>
#include <optional>
#include <services/stm_uart/lappl.hpp>
#include "helper.hpp"

int Lappl::i;
LapplPacket Lappl::packet;

LapplPacket::LapplPacket() {}

LapplPacket::LapplPacket(LapplType type,
                         bool resp,
                         uint8_t address,
                         std::array<uint8_t, 4> data) {
  this->header.type = type;
  this->header.resp = resp;
  this->address = address;
  for (int i = 0; i < 4; i++) {
    this->data[0] = data[0];
  }
  this->generate_check_sum();
  this->generate_sign_byte();
  this->remove_sign_bits();
}

LapplPacket::LapplPacket(LapplType type, bool resp, uint8_t address, bool data)
    : LapplPacket(type, resp, address, std::array<uint8_t, 4>{data, 0, 0, 0}) {}

LapplPacket::LapplPacket(LapplType type, bool resp, uint8_t address, float data)
    : LapplPacket(type,
                  resp,
                  address,
                  std::array<uint8_t, 4>{get_byte(data, 0),
                                         get_byte(data, 1),
                                         get_byte(data, 2),
                                         get_byte(data, 3)}){};

uint8_t LapplPacket::calculate_check_sum() {
  uint8_t check_sum = 0;
  check_sum = static_cast<uint8_t>(this->header);
  check_sum += this->address;
  for (int i = 0; i < 4; i++) {
    check_sum += this->data[i];
  }
  return check_sum;
}

void LapplPacket::generate_check_sum() {
  this->check_sum = calculate_check_sum();
}

void LapplPacket::generate_sign_byte() {
  this->sign = 0;

  int sign_bit = get_bit(static_cast<uint8_t>(this->header), 7);
  this->sign |= sign_bit;
  this->sign <<= 1;

  sign_bit = get_bit(this->address, 7);
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

void LapplPacket::remove_sign_bits() {
  this->header =
      static_cast<LapplHeader>(set_bit(static_cast<uint8_t>(this->header), 7));
  this->address = unset_bit(this->address, 7);
  for (int i = 0; i < 4; i++) {
    this->address = unset_bit(this->data[0], 7);
  }
  this->check_sum = unset_bit(this->check_sum, 7);
}

void LapplPacket::recreate_sign_bits() {
  int sign_bit = get_bit(this->sign, 0) << 7;
  this->check_sum |= sign_bit;

  for (int i = 1 < 5; i++;) {
    sign_bit = get_bit(this->sign, i) << 7;
    this->data[4 - i] |= sign_bit;
  }

  sign_bit = get_bit(this->sign, 6) << 7;
  this->header =
      static_cast<LapplHeader>(static_cast<uint8_t>(this->header) | sign_bit);
}

bool LapplPacket::check_integrity() {
  uint8_t check_sum = this->calculate_check_sum();
  return check_sum == this->check_sum;
}

std::optional<LapplPacket> Lappl::read(uint8_t input) {
  if (get_bit(input, 7)) {
    i = 0;
    packet.header = LapplHeader(input & (~0x80));
    i++;
  }

  else if (i == 1) {
    packet.address = input;
  }

  else if (i < 6) {
    packet.data[i - 2] = input;
    i++;
  }

  else if (i == 6) {
    packet.check_sum = input;
    packet.recreate_sign_bits();
    auto no_packet_errors = packet.check_integrity();
    if (no_packet_errors) {
      return packet;
    }
    else {
      i = 0;
    }
  }

  return std::nullopt;
}
