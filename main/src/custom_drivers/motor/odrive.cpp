#include "custom_drivers/motor/odrive.hpp"
#include <cmath>
#include <cstring>
#include <format>
#include "custom_drivers/can_device_reader/can_packet.hpp"
#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive/command.hpp"
#include "esp_log.h"
#include "esp_twai_types.h"
#include "hal/twai_types.h"
#include "jaythread/sync.hpp"

ODriveMotorDriver::ODriveMotorDriver(int id,
                                     twai_node_handle_t twai,
                                     float max_torque,
                                     MotorDirection direction,
                                     float torque_constant)
    : AbstractMotorDriver(id, twai, max_torque, direction, torque_constant) {}

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
  packet.header = make_header(ODriveMotorCommand::ABSOLUTE_POSITION);
  packet.header.dlc = 4;
  float zero = -1;
  memcpy(packet.data.data(), &zero, 4);
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

void ODriveMotorDriver::enable() {
  send_set_torque_mode_command();
  Sync::sleep(10);
  send_set_axis_state_command(ODriveMotorAxisState::CLOSED_LOOP_CONTROL);
}

void ODriveMotorDriver::disable() {
  send_set_axis_state_command(ODriveMotorAxisState::IDLE);
}

void ODriveMotorDriver::zero_pos() {
  position_offset = feedback.position;
}

void ODriveMotorDriver::consume(CanPacket packet) {
  auto command = packet.header.id & ((1 << 5) - 1);
  switch (static_cast<ODriveMotorCommand>(command)) {
    case ODriveMotorCommand::GET_ENCODER_ESTIMATES: {
      float* raw_position = reinterpret_cast<float*>(&packet.data.data()[0]);
      float* raw_velocity = reinterpret_cast<float*>(&packet.data.data()[4]);
      consume_position(*raw_position);
      consume_velocity(*raw_velocity);
      position_valid_sem.give();
      break;
    }
    case ODriveMotorCommand::HEARTBEAT:
      break;
    default:
      ESP_LOGW(tag.c_str(),
               "unhandled command received 0x%02x value: %f",
               command,
               *(float*)(packet.data.data()));
      break;
  }
}

float ODriveMotorDriver::get_position() {
  return feedback.position - position_offset;
}

void ODriveMotorDriver::init() {
  ESP_LOGI(tag.c_str(), "initializing");
  disable();
  Sync::sleep(10);
  enable();
  Sync::sleep(10);
  set_torque(0);
  Sync::sleep(10);
  ESP_LOGI(tag.c_str(), "waiting for position feedback");
  position_valid_sem.clear();
  position_valid_sem.take();
  Sync::sleep(2000);
  if (feedback.position == 0) {
    ESP_LOGW(tag.c_str(),
             "position feedback is 0, this may be the result of not waiting long enough for "
             "position feedback");
  } else {
    ESP_LOGI(tag.c_str(), "position feedback is %f for zero posing", feedback.position);
  }
  zero_pos();
  disable();
  Sync::sleep(10);
  ESP_LOGI(tag.c_str(), "zero pos completed");
  ESP_LOGI(tag.c_str(), "initialization completed");
}