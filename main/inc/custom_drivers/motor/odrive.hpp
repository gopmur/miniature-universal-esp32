#pragma once

#include "custom_drivers/can_device_reader.hpp"
#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive/command.hpp"

#include "jaythread/ipc/binary_semaphore.hpp"

class ODriveMotorFeedbackReader : public AbstractCanDeviceReader {
  void consume(CanPacket packet);
};

class ODriveMotorDriver : public AbstractMotorDriver {
  MAKE_LOGGABLE("odrive");

  private:
  float position_offset = 0;
  int get_packet_id(ODriveMotorCommand command);
  twai_frame_header_t make_header(ODriveMotorCommand command);
  BinarySemaphore position_valid_sem;

  CanPacket make_torque_packet(float torque);
  CanPacket make_read_encoder_packet();
  CanPacket make_enable_packet();
  CanPacket make_disable_packet();
  CanPacket make_zero_pos_packet();

  CanPacket make_clean_errors_packet();
  CanPacket make_set_torque_mode_packet();
  CanPacket make_set_axis_state_packet(ODriveMotorAxisState axis_state);

  void send_set_torque_mode_command();
  void send_set_axis_state_command(ODriveMotorAxisState axis_state);

  public:
  void enable();
  void disable();
  void zero_pos();
  void consume(CanPacket packet);
  float get_position();
  void init();
  using AbstractMotorDriver::AbstractMotorDriver;
};
