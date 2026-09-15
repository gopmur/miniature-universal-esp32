#include "tasks/motor.hpp"
#include "esp_log.h"
#include "jaythread/sync.hpp"

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
  while (true) {
    right_motor->set_torque(right_torque);
    left_motor->set_torque(left_torque);
    Sync::sleep(10);
  }
}