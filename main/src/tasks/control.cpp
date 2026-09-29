#include "tasks/control.hpp"
#include <cmath>
#include "controller.hpp"
#include "custom_drivers/motor.hpp"
#include "esp_log.h"
#include "helper/formats.hpp"
#include "jaythread/sync.hpp"
#include "system_logger.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;

void ControlTask::reset() {
  manual_controller.reset();
  automatic_controller.reset();
  semiautomatic_controller.reset();
  smart_controller.reset();
}

void ControlTask::main() {
  ControllerInput input;
  ControllerOutput output;
  const size_t left_motor_index = 0;
  const size_t right_motor_index = 1;
  while (true) {
    if (prev_control_mod != control_mode) {
      reset();
      prev_control_mod = control_mode;
      LOGI("entered mode %s", get_control_mode_string(control_mode));
    }
    input.left_motor.position = motor_task->get_position(left_motor_index);
    input.right_motor.position = motor_task->get_position(right_motor_index);
    input.left_motor.velocity = motor_task->get_velocity(left_motor_index);
    input.right_motor.velocity = motor_task->get_velocity(right_motor_index);
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
        case ControlMode::SMART:
          output = smart_controller.run(input);
          break;
        default:
          output = zero_controller.run(input);
          LOGW("unhandled control mod %d", static_cast<uint32_t>(control_mode));
      }
    } else {
      output = zero_controller.run(input);
    }
    motor_task->set_torque(output.left_motor.torque, output.right_motor.torque);
    Sync::sleep(10);
  }
}