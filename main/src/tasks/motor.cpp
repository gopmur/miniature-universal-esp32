#include "tasks/motor.hpp"
#include "esp_timer.h"
#include "jaythread/sync.hpp"
#include "system_logger.hpp"

MotorTask::MotorTask(AbstractMotorDriver* left_motor, AbstractMotorDriver* right_motor)
    : left_motor(left_motor), right_motor(right_motor) {}

void MotorTask::set_left_torque(float torque) {
  left_torque = torque;
}
void MotorTask::set_right_torque(float torque) {
  right_torque = torque;
}
void MotorTask::set_torque(float left_torque, float right_torque) {
  set_left_torque(left_torque);
  set_right_torque(right_torque);
}

void MotorTask::enable() {
  enable_left();
  enable_right();
}

void MotorTask::disable() {
  disable_left();
  disable_right();
}

void MotorTask::enable_left() {
  left_torque = 0;
  left_motor->set_torque(0);
  left_motor->enable();
  left_motor->set_torque(0);
}

void MotorTask::disable_left() {
  left_torque = 0;
  left_motor->set_torque(0);
  left_motor->disable();
  left_motor->set_torque(0);
}

void MotorTask::enable_right() {
  right_torque = 0;
  right_motor->set_torque(0);
  right_motor->enable();
  right_motor->set_torque(0);
}

void MotorTask::disable_right() {
  right_torque = 0;
  right_motor->set_torque(0);
  right_motor->disable();
  right_motor->set_torque(0);
}

void MotorTask::main() {
  left_motor->init();
  right_motor->init();
  disable();
  int64_t left_esp_pending_start_timestamp_us = 0;
  bool left_pending_timer_started = false;
  int64_t right_esp_pending_start_timestamp_us = 0;
  bool right_pending_timer_started = false;
  while (true) {
    if (left_motor->pending & !left_pending_timer_started) {
      left_esp_pending_start_timestamp_us = esp_timer_get_time();
      left_pending_timer_started = true;
    } else if (left_motor->pending) {
      auto now_us = esp_timer_get_time();
      if (((now_us - left_esp_pending_start_timestamp_us) / 1000) > 1000) {
        LOGE("left motor timeout occurred! check motor connection");
      }
    } else {
      left_pending_timer_started = false;
    }
    if (right_motor->pending & !right_pending_timer_started) {
      right_esp_pending_start_timestamp_us = esp_timer_get_time();
      right_pending_timer_started = true;
    } else if (right_motor->pending) {
      auto now_us = esp_timer_get_time();
      if (((now_us - right_esp_pending_start_timestamp_us) / 1000) > 1000) {
        LOGE("right motor timeout occurred! check motor connection");
      }
    } else {
      right_pending_timer_started = false;
    }
    right_motor->set_torque(right_torque);
    left_motor->set_torque(left_torque);
    Sync::sleep(10);
  }
}