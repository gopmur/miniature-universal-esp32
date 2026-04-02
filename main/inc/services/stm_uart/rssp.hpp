#pragma once

#include <array>
#include <cstdint>
#include <optional>

enum class RsspAddress : uint8_t {
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
  ADDRESS_COUNT,
};

enum class RsspCommand : uint8_t { RESTART };

enum class RsspType : uint8_t {
  WRITE,
  READ,
  START_STREAM,
  STOP_STREAM,
  EOC,  // end of cycle
  COMMAND
};

struct _RsspHeader {
  RsspType type : 3;  // bits 6..5
  uint8_t ack : 1;    // bit 4
  uint8_t resp : 1;   // bit 3
  uint8_t _res : 2;   // bits 2..0 (must be zero) maybe can be used for version
                      // control ???
};

union RsspHeader {
  uint8_t u8;
  _RsspHeader b;
};

struct RsspPacket {
  RsspHeader header;
  uint8_t sign;
  RsspAddress address;
  uint8_t data[4];
  uint8_t check_sum;

  static RsspPacket make_read_response_packet(RsspAddress address,
                                              std::array<uint8_t, 4> data);
  static RsspPacket make_read_response_packet(RsspAddress address, bool data);
  static RsspPacket make_read_response_packet(RsspAddress address, float data);
  static RsspPacket make_read_response_packet(RsspAddress address,
                                              uint8_t data);
  static RsspPacket make_read_packet(RsspAddress address);
  static RsspPacket make_write_packet(RsspAddress address,
                                      std::array<uint8_t, 4> data);
  static RsspPacket make_write_packet(RsspAddress address, bool data);
  static RsspPacket make_write_packet(RsspAddress address, float data);
  static RsspPacket make_write_packet(RsspAddress address, uint8_t data);
  static RsspPacket make_write_packet(RsspAddress address, uint16_t data);
  static RsspPacket make_write_packet(RsspAddress address, uint32_t data);
  static RsspPacket make_write_packet(RsspAddress address, int8_t data);
  static RsspPacket make_write_packet(RsspAddress address, int16_t data);
  static RsspPacket make_write_packet(RsspAddress address, int32_t data);
  static RsspPacket make_start_stream_packet(RsspAddress address);
  static RsspPacket make_stop_stream_packet(RsspAddress address);
  static RsspPacket make_eoc_packet();
  static RsspPacket make_command_packet(RsspCommand command);

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

class Rssp {
  private:
  static int i;
  static RsspPacket packet;

  public:
  static std::optional<RsspPacket> read_stream(uint8_t rssp_byte);
};