#pragma once

#include <array>
#include <cstdint>
#include <optional>

enum class LapplType : uint8_t {
  WRITE,
  READ,
  START_STREAM,
  STOP_STREAM,
};

struct LapplHeader {
  LapplType type : 2;  // bits 6..5
  uint8_t ack : 1;     // bit 4
  uint8_t resp : 1;    // bit 3
  uint8_t _res : 3;    // bits 2..0 (must be zero)

  constexpr LapplHeader() : type(LapplType::WRITE), ack(0), resp(0), _res(0) {}

  explicit constexpr LapplHeader(uint8_t v)
      : type(static_cast<LapplType>((v >> 5) & 0x03)),
        ack((v >> 4) & 0x01),
        resp((v >> 3) & 0x01),
        _res(0) {}

  constexpr explicit operator uint8_t() const {
    return ((static_cast<uint8_t>(type) & 0x03) << 5) | ((ack & 0x01) << 4) |
           ((resp & 0x01) << 3);
  }
};

struct LapplPacket {
  LapplHeader header;
  uint8_t sign;
  uint8_t address;
  uint8_t data[4];
  uint8_t check_sum;
  
  LapplPacket();
  LapplPacket(LapplType type, bool resp, uint8_t address, std::array<uint8_t, 4> data);
  LapplPacket(LapplType type, bool resp, uint8_t address, bool data);
  LapplPacket(LapplType type, bool resp, uint8_t address, float data);
  
  void recreate_sign_bits();
  bool check_integrity();
  
  private:
  uint8_t calculate_check_sum();
  void generate_check_sum();
  void generate_sign_byte();
  void remove_sign_bits();
};

class Lappl {
  private:
  static int i;
  static LapplPacket packet;
  
  static std::optional<LapplPacket> read(uint8_t lappl_byte);
};