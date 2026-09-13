#include "custom_drivers/motor.hpp"
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_twai.h"
#include "hal/twai_types.h"

AbstractMotorDriver::AbstractMotorDriver(int id,
                                         twai_node_handle_t twai,
                                         float max_torque,
                                         MotorDirection direction,
                                         float torque_constant)
    : id(id),
      twai(twai),
      max_torque(max_torque),
      direction(direction),
      torque_constant(torque_constant) {
  tag = "main" + std::to_string(id);
  if (max_torque < 0) {
    ESP_LOGE(tag.c_str(),
             "max_torque is set to %f, max_torque can only be a positive value",
             max_torque);
    abort();
  }
}

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

void AbstractMotorDriver::set_torque(float torque) {
  torque *= torque_constant;
  if (direction == MotorDirection::BACKWARD) {
    torque = -torque;
  }
  int torque_sign = torque >= 0 ? 1 : -1;
  if (fabs(torque) > max_torque) {
    ESP_LOGW(tag.c_str(),
             "applied torque is %f which is passed the secured limit %f",
             torque,
             max_torque);
    torque = max_torque * torque_sign;
  }
  MotorPacket packet = make_torque_packet(torque);
  send_packet(packet);
}

void AbstractMotorDriver::zero_pos() {
  auto packet = make_zero_pos_packet();
  send_packet(packet);
}

void AbstractMotorDriver::enable() {
  MotorPacket packet = make_enable_packet();
  ESP_LOGI(tag.c_str(), "enabled");
  send_packet(packet);
}

void AbstractMotorDriver::disable() {
  MotorPacket packet = make_disable_packet();
  ESP_LOGI(tag.c_str(), "disabled");
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

void AbstractMotorDriver::poll_encoder() {
  MotorPacket packet = make_read_encoder_packet();
  send_packet(packet);
}

float AbstractMotorDriver::get_torque() {
  return feedback.torque;
}
float AbstractMotorDriver::get_position() {
  return feedback.position;
}
float AbstractMotorDriver::get_velocity() {
  return feedback.velocity;
}
float AbstractMotorDriver::get_temperature() {
  return feedback.temperature;
}
