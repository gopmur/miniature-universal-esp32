#pragma once

#include <array>
#include <cstdint>
#include <optional>

enum class LapplAddress : uint8_t {
  ZERO,
  RUNNING,
  LEFT_TORQUE,
  RIGHT_TORQUE,
  CONTROL_MODE,
  IMU_GX,
  IMU_GY,
  IMU_GZ,
  LED_SERVICE_CPU_USAGE,
  IMU_SERVICE_CPU_USAGE,
  ESP_UART_TX_SERVICE_CPU_USAGE,
  ESP_UART_RX_SERVICE_CPU_USAGE,
  MOTOR_SERVICE_CPU_USAGE,
  CAN_RECV_SERVICE_CPU_USAGE,
  SD_SERVICE_CPU_USAGE,
  MONITOR_SERVICE_CPU_USAGE,
  TEST_RANDOM,
  TEST_ZERO,
  ADDRESS_COUNT,
};

enum class LapplCommand : uint8_t { RESTART };

enum class LapplType : uint8_t {
  WRITE,
  READ,
  START_STREAM,
  STOP_STREAM,
  EOC,  // end of cycle
  COMMAND
};

struct _LapplHeader {
  LapplType type : 3;  // bits 6..5
  uint8_t ack : 1;     // bit 4
  uint8_t resp : 1;    // bit 3
  uint8_t _res : 2;    // bits 2..0 (must be zero) maybe can be used for version
                       // control ???
};

union LapplHeader {
  uint8_t u8;
  _LapplHeader b;
};

struct LapplPacket {
  LapplHeader header;
  uint8_t sign;
  LapplAddress address;
  uint8_t data[4];
  uint8_t check_sum;

  static LapplPacket make_read_response_packet(LapplAddress address,
                                               std::array<uint8_t, 4> data);
  static LapplPacket make_read_response_packet(LapplAddress address, bool data);
  static LapplPacket make_read_response_packet(LapplAddress address,
                                               float data);
  static LapplPacket make_read_response_packet(LapplAddress address,
                                               uint8_t data);
  static LapplPacket make_read_packet(LapplAddress address);
  static LapplPacket make_write_packet(LapplAddress address,
                                       std::array<uint8_t, 4> data);
  static LapplPacket make_write_packet(LapplAddress address, bool data);
  static LapplPacket make_write_packet(LapplAddress address, float data);
  static LapplPacket make_write_packet(LapplAddress address, uint8_t data);
  static LapplPacket make_start_stream_packet(LapplAddress address);
  static LapplPacket make_stop_stream_packet(LapplAddress address);
  static LapplPacket make_eoc_packet();
  static LapplPacket make_command_packet(LapplCommand command);

  float get_float();
  uint8_t get_uint8();
  bool get_bool();

  void recreate_sign_bits();
  bool check_integrity();
  std::array<uint8_t, 8> get_raw_packet();

  private:
  uint8_t calculate_check_sum();
  void generate_check_sum();
  void generate_sign_byte();
  void remove_sign_bits();
  void pack();
};

class Lappl {
  private:
  static int i;
  static LapplPacket packet;

  public:
  static std::optional<LapplPacket> read_stream(uint8_t lappl_byte);
};