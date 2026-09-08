#pragma once

#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive/command.hpp"
#include "custom_drivers/motor/packet.hpp"

class ODriveMotorDriver : public AbstractMotorDriver {
  private:
  int get_packet_id(ODriveMotorCommand command);
  twai_frame_header_t make_header(ODriveMotorCommand command);

  MotorPacket make_torque_packet(float torque);
  MotorPacket make_read_encoder_packet();
  MotorPacket make_enable_packet();
  MotorPacket make_disable_packet();
  MotorPacket make_zero_pos_packet();

  MotorPacket make_clean_errors_packet();
  MotorPacket make_set_torque_mode_packet();
  MotorPacket make_set_axis_state_packet(ODriveMotorAxisState axis_state);

  void send_set_torque_mode_command();
  void send_set_axis_state_command(ODriveMotorAxisState axis_state);

  
  public:
  void send_enable_command();
  void send_disable_command();
  ODriveMotorDriver(int id, twai_node_handle_t twai);
};
