#pragma once

#include "custom_drivers/motor.hpp"

class SmcMotorDriver : public AbstractMotorDriver {
  private:
  CanPacket make_torque_packet(float torque);
  CanPacket make_read_encoder_packet();
  CanPacket make_enable_packet();
  CanPacket make_disable_packet();
  CanPacket make_zero_pos_packet();
  static int torque_float_to_uint(float x, int bits);

  public:
  using AbstractMotorDriver::AbstractMotorDriver;
};
