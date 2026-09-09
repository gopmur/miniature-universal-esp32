#pragma once

#include "custom_drivers/can_device_reader.hpp"
#include "custom_drivers/motor/packet.hpp"
#include "esp_twai_types.h"
#include "hal/twai_types.h"

class AbstractMotorDriver : public AbstractCanDeviceReader {
  protected:
  int id;
  twai_node_handle_t twai;

  twai_frame_header_t make_header();
  virtual MotorPacket make_torque_packet(float tau_ff_in) = 0;
  virtual MotorPacket make_read_encoder_packet() = 0;
  virtual MotorPacket make_enable_packet() = 0;
  virtual MotorPacket make_disable_packet() = 0;
  virtual MotorPacket make_zero_pos_packet() = 0;
  void send_packet(MotorPacket packet);

  public:
  AbstractMotorDriver(int id, twai_node_handle_t twai);
  void send_torque_command(float torque);
  void send_read_encoder_command();
  void send_enable_command();
  void send_disable_command();
  void send_zero_pos_command();
};
