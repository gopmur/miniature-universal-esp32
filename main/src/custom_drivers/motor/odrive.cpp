#include "custom_drivers/motor/odrive.hpp"
#include <cmath>
#include <cstring>
#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive/command.hpp"
#include "esp_log.h"
#include "esp_twai_types.h"
#include "hal/twai_types.h"
#include "jaythread/sync.hpp"

ODriveMotorDriver::ODriveMotorDriver(int id, twai_node_handle_t twai)
    : AbstractMotorDriver(id, twai) {}

int ODriveMotorDriver::get_packet_id(ODriveMotorCommand command) {
  return (id << 5) | static_cast<int>(command);
}

twai_frame_header_t ODriveMotorDriver::make_header(ODriveMotorCommand command) {
  twai_frame_header_t header = AbstractMotorDriver::make_header();
  header.id = get_packet_id(command);
  return header;
}

MotorPacket ODriveMotorDriver::make_clean_errors_packet() {
  MotorPacket packet;
  auto header = make_header(ODriveMotorCommand::CLEAR_ERRORS);
  header.dlc = 0;
  packet.header = header;
  return packet;
}

MotorPacket ODriveMotorDriver::make_set_torque_mode_packet() {
  MotorPacket packet;
  packet.header = make_header(ODriveMotorCommand::SET_CONTROLLER_MODES);
  packet.header.dlc = 8;
  uint32_t control_mode = 1;
  uint32_t input_mode = 1;
  memcpy(&packet.data.data()[0], &control_mode, 4);
  memcpy(&packet.data.data()[4], &input_mode, 4);
  return packet;
}
MotorPacket ODriveMotorDriver::make_set_axis_state_packet(ODriveMotorAxisState axis_state) {
  MotorPacket packet;
  packet.header = make_header(ODriveMotorCommand::SET_AXIS_STATE);
  packet.header.dlc = 4;
  memcpy(packet.data.data(), reinterpret_cast<uint32_t*>(&axis_state), 4);
  return packet;
}

MotorPacket ODriveMotorDriver::make_torque_packet(float torque) {
  MotorPacket packet;
  packet.header = make_header(ODriveMotorCommand::SET_INPUT_TORQUE);
  packet.header.dlc = 8;
  memcpy(packet.data.data(), &torque, 4);
  return packet;
}

MotorPacket ODriveMotorDriver::make_read_encoder_packet() {
  MotorPacket packet;
  return packet;
};

MotorPacket ODriveMotorDriver::make_enable_packet() {
  MotorPacket packet;
  return packet;
}

MotorPacket ODriveMotorDriver::make_disable_packet() {
  MotorPacket packet;
  return packet;
}

MotorPacket ODriveMotorDriver::make_zero_pos_packet() {
  MotorPacket packet;
  return packet;
}

void ODriveMotorDriver::send_set_torque_mode_command() {
  auto packet = make_set_torque_mode_packet();
  send_packet(packet);
}

void ODriveMotorDriver::send_set_axis_state_command(ODriveMotorAxisState axis_state) {
  auto packet = make_set_axis_state_packet(axis_state);
  send_packet(packet);
}

void ODriveMotorDriver::send_enable_command() {
  send_set_torque_mode_command();
  Sync::sleep(10);
  send_set_axis_state_command(ODriveMotorAxisState::CLOSED_LOOP_CONTROL);
}

void ODriveMotorDriver::send_disable_command() {
  send_set_axis_state_command(ODriveMotorAxisState::IDLE);
}
