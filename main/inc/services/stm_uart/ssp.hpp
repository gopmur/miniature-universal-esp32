#pragma once

#include <array>
#include <cstdint>
#include <optional>

enum class SspAddress : uint8_t {
  ZERO,
  RAND,
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
  LED_SERVICE_MIN_STACK_FREE,
  IMU_SERVICE_MIN_STACK_FREE,
  ESP_UART_TX_SERVICE_MIN_STACK_FREE,
  ESP_UART_RX_SERVICE_MIN_STACK_FREE,
  MOTOR_SERVICE_MIN_STACK_FREE,
  CAN_RECV_SERVICE_MIN_STACK_FREE,
  SD_SERVICE_MIN_STACK_FREE,
  MONITOR_SERVICE_MIN_STACK_FREE,
  LED_SERVICE_STACK_SIZE,
  IMU_SERVICE_STACK_SIZE,
  ESP_UART_TX_SERVICE_STACK_SIZE,
  ESP_UART_RX_SERVICE_STACK_SIZE,
  MOTOR_SERVICE_STACK_SIZE,
  CAN_RECV_SERVICE_STACK_SIZE,
  SD_SERVICE_STACK_SIZE,
  MONITOR_SERVICE_STACK_SIZE,
  HEAP_FREE,
  MIN_HEAP_FREE,
  HEAP_SIZE,
  RTC_TIME,
  RTC_DATE,
  MOTOR_POS_LEFT,
  MOTOR_POS_RIGHT,
  AUTOMATIC_LEFT_TIMEOUT,
  AUTOMATIC_LEFT_TORQUE,
  AUTOMATIC_LEFT_VELOCITY_THRESHOLD,
  AUTOMATIC_RIGHT_TIMEOUT,
  AUTOMATIC_RIGHT_TORQUE,
  AUTOMATIC_RIGHT_VELOCITY_THRESHOLD,
  ADDRESS_COUNT,
};

enum class SspCommand : uint8_t {
  RESTART,
  START_REPORT,
  STOP_REPORT,
};

enum class SspType : uint8_t {
  WRITE,
  READ,
  START_STREAM,
  STOP_STREAM,
  EOC,  // end of cycle
  COMMAND
};

struct _SspHeader {
  SspType type : 3;  // bits 6..5
  uint8_t ack : 1;   // bit 4
  uint8_t resp : 1;  // bit 3
  uint8_t _res : 2;  // bits 2..0 (must be zero) maybe can be used for version
                     // control ???
};

union SspHeader {
  uint8_t u8;
  _SspHeader b;
};

struct SspPacket {
  SspHeader header;
  uint8_t sign;
  SspAddress address;
  uint8_t data[4];
  uint8_t check_sum;

  static SspPacket make_read_response_packet(SspAddress address, std::array<uint8_t, 4> data);
  static SspPacket make_read_response_packet(SspAddress address, bool data);
  static SspPacket make_read_response_packet(SspAddress address, float data);
  static SspPacket make_read_response_packet(SspAddress address, uint8_t data);
  static SspPacket make_read_packet(SspAddress address);
  static SspPacket make_write_packet(SspAddress address, std::array<uint8_t, 4> data);
  static SspPacket make_write_packet(SspAddress address, bool data);
  static SspPacket make_write_packet(SspAddress address, float data);
  static SspPacket make_write_packet(SspAddress address, uint8_t data);
  static SspPacket make_write_packet(SspAddress address, uint16_t data);
  static SspPacket make_write_packet(SspAddress address, uint32_t data);
  static SspPacket make_write_packet(SspAddress address, int8_t data);
  static SspPacket make_write_packet(SspAddress address, int16_t data);
  static SspPacket make_write_packet(SspAddress address, int32_t data);
  static SspPacket make_start_stream_packet(SspAddress address);
  static SspPacket make_stop_stream_packet(SspAddress address);
  static SspPacket make_eoc_packet();
  static SspPacket make_command_packet(SspCommand command);

  float get_float();
  uint8_t get_uint8();
  bool get_bool();
  uint32_t get_uint32();
  uint16_t get_uint16();

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

class Ssp {
  private:
  static int i;
  static SspPacket packet;

  public:
  static std::optional<SspPacket> read_stream(uint8_t ssp_byte);
};