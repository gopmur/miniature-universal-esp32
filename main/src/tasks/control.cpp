#include "tasks/control.hpp"
#include <cmath>
#include "controller.hpp"
#include "custom_drivers/motor.hpp"
#include "esp_log.h"
#include "jaythread/sync.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;
extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;

void ControlTask::reset() {
  manual_controller.reset();
  automatic_controller.reset();
  semiautomatic_controller.reset();
}

void ControlTask::main() {
  ControllerInput input;
  ControllerOutput output;
  while (true) {
    if (prev_control_mod != control_mode) {
      reset();
      prev_control_mod = control_mode;
    }
    input.left_motor.position = left_motor->get_position();
    input.right_motor.position = right_motor->get_position();
    input.left_motor.velocity = left_motor->get_velocity();
    input.right_motor.velocity = right_motor->get_velocity();
    if (running) {
      switch (control_mode) {
        case ControlMode::MANUAL:
          output = manual_controller.run(input);
          break;
        case ControlMode::AUTO:
          output = automatic_controller.run(input);
          break;
        case ControlMode::SEMI_AUTO:
          output = semiautomatic_controller.run(input);
          break;
        default:
          output = zero_controller.run(input);
          ESP_LOGW(tag.c_str(), "unhandled control mod %d", static_cast<uint32_t>(control_mode));
      }
    } else {
      output = zero_controller.run(input);
    }
    motor_task->set_torque(output.left_motor.torque, output.right_motor.torque);
    Sync::sleep(10);
  }
}