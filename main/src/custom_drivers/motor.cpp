#include "custom_drivers/motor.hpp"
#include <stdlib.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include "esp_err.h"
#include "esp_twai.h"
#include "hal/twai_types.h"
#include "system_logger.hpp"

void MotorTimeoutTimer::main() {
  motor->status_mutex.take();
  if (motor->status != MotorStatus::TIMEDOUT) {
    LOGE("motor 0x%02x timed out", motor->id);
  }
  motor->status = MotorStatus::TIMEDOUT;
  motor->status_mutex.give();
}

void MotorTimeoutTimer::init(AbstractMotorDriver* motor) {
  auto timer_name = std::format("{:#02x}_motor_tim", motor->id);
  this->motor = motor; 
  Timer::init(timer_name.c_str(), 1000, false);
}

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
  if (max_torque < 0) {
    LOGE("0x%02x max_torque is set to %f, max_torque can only be a positive value", id, max_torque);
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
  torque = apply_direction(torque);
  int torque_sign = torque >= 0 ? 1 : -1;
  if (fabs(torque) > max_torque) {
    LOGW("0x%02x applied torque is %f which is passed the secured limit %f",
         id,
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
  LOGI("0x%02x enabled", id);
  send_packet(packet);
}

void AbstractMotorDriver::disable() {
  MotorPacket packet = make_disable_packet();
  LOGI("0x%02x disabled", id);
  send_packet(packet);
}

void AbstractMotorDriver::send_packet(MotorPacket packet) {
  timeout_timer.start(1000);
  twai_frame_t twai_frame;
  twai_frame.header = packet.header;
  twai_frame.buffer = packet.data.data();
  twai_frame.buffer_len =
      std::min(static_cast<uint32_t>(packet.header.dlc), static_cast<uint32_t>(packet.data.size()));
  auto status = twai_node_transmit(twai, &twai_frame, 1);
  if (status == ESP_ERR_TIMEOUT) {
    LOGW("timeout occurred while queueing id 0x%02x", packet.header.id);
  } else if (status != ESP_OK) {
    LOGE("unhandled error occurred while queueing id 0x%02x. %s",
         packet.header.id,
         esp_err_to_name(status));
  }
  status = twai_node_transmit_wait_all_done(twai, 100);
  if (status == ESP_ERR_TIMEOUT) {
    LOGW("timeout occurred while transmiting id 0x%02x. check the physical connection",
         packet.header.id);
  } else if (status != ESP_OK) {
    LOGE("unhandled error occurred while transmiting id 0x%02x. %s",
         packet.header.id,
         esp_err_to_name(status));
  }
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

float AbstractMotorDriver::apply_direction(float value) {
  return direction == MotorDirection::BACKWARD ? -value : value;
}

void AbstractMotorDriver::consume_position(float position) {
  feedback.position = apply_direction(position);
}

void AbstractMotorDriver::consume_velocity(float velocity) {
  feedback.velocity = apply_direction(velocity);
}

void AbstractMotorDriver::consume_torque(float torque) {
  feedback.torque = apply_direction(torque);
}

void AbstractMotorDriver::init() {
  timeout_timer.init(this);
}

void AbstractMotorDriver::consume(CanPacket packet) {
  timeout_timer.stop_block();
  status_mutex.take();
  if (status != MotorStatus::OK) {
    LOGI("motor 0x%02x changed status to ok", id);
  }
  status = MotorStatus::OK;
  status_mutex.give();
}

MotorStatus AbstractMotorDriver::get_status() {
  return status;
}