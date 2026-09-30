#pragma once

#include "custom_drivers/motor.hpp"

class TMotorDriver : public AbstractMotorDriver {
  private:
  CanPacket make_torque_packet(float torque);
  CanPacket make_read_encoder_packet();
  CanPacket make_enable_packet();
  CanPacket make_disable_packet();
  CanPacket make_zero_pos_packet();
  static int float_to_uint(float x, float x_min, float x_max, int bits);

  public:
  using AbstractMotorDriver::AbstractMotorDriver;
};
