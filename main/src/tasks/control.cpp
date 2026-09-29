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
  hexa_manual_controller.reset();
  hexa_automatic_controller.reset();
  hexa_semiautomatic_controller.reset();
  hexa_smart_controller.reset();
}

void ControlTask::main() {
  ControllerInput input;
  ControllerOutput output;
  while (true) {
    if (prev_control_mod != control_mode) {
      reset();
      prev_control_mod = control_mode;
      LOGI("entered mode %s", get_control_mode_string(control_mode));
    }

    for (size_t i = 0; i < motor_task->motor_count; i++) {
      input.motor_feedbacks[i].position = motor_task->get_position(i);
      input.motor_feedbacks[i].velocity = motor_task->get_velocity(i);
    }
    if (running) {
      switch (control_mode) {
        case ControlMode::HEXA_MANUAL:
          output = hexa_manual_controller.run(input);
          break;
        case ControlMode::HEXA_AUTOMATIC:
          output = hexa_automatic_controller.run(input);
          break;
        case ControlMode::HEXA_SEMIAUTOMATIC:
          output = hexa_semiautomatic_controller.run(input);
          break;
        case ControlMode::HEXA_SMART:
          output = hexa_smart_controller.run(input);
          break;
        default:
          output = zero_controller.run(input);
          LOGW("unhandled control mod %d", static_cast<uint32_t>(control_mode));
      }
    } else {
      output = zero_controller.run(input);
    }
    for (int i = 0; i < motor_task->motor_count; i++) {
      motor_task->set_torque(i, output.torques[i]);
    }
    Sync::sleep(10);
  }
}