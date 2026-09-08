#include "custom_drivers/motor.hpp"
#include <algorithm>
#include "esp_twai.h"
#include "hal/twai_types.h"

AbstractMotorDriver::AbstractMotorDriver(int id, twai_node_handle_t twai) : id(id), twai(twai) {}

twai_frame_header_t AbstractMotorDriver::make_header() {
  twai_frame_header_t header = {
      .id = static_cast<uint32_t>(id),
      .dlc = 8,
      .brs = 0,
      .esi = 0,
      .fdf = 0,
      .ide = 0,
      .rtr = 0,
      .timestamp = 0,
  };
  return header;
}

void AbstractMotorDriver::send_torque_command(float tau_ff_in) {
  MotorPacket packet = make_torque_packet(tau_ff_in);
  send_packet(packet);
}

void AbstractMotorDriver::send_zero_pos_command() {
  auto packet = make_zero_pos_packet();
  send_packet(packet);
}

void AbstractMotorDriver::send_enable_command() {
  MotorPacket packet = make_enable_packet();
  send_packet(packet);
}

void AbstractMotorDriver::send_disable_command() {
  MotorPacket packet = make_disable_packet();
  send_packet(packet);
}

void AbstractMotorDriver::send_packet(MotorPacket packet) {
  twai_frame_t twai_frame;
  twai_frame.header = packet.header;
  twai_frame.buffer = packet.data.data();
  twai_frame.buffer_len =
      std::min(static_cast<uint32_t>(packet.header.dlc), static_cast<uint32_t>(packet.data.size()));
  ESP_ERROR_CHECK(twai_node_transmit(twai, &twai_frame, -1));
  ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(twai, -1));
}

void AbstractMotorDriver::send_read_encoder_command() {
  MotorPacket packet = make_read_encoder_packet();
  send_packet(packet);
}
