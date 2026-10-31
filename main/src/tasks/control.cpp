#include "tasks/control.hpp"
#include <cmath>
#include "controller.hpp"
#include "helper/ds/period.hpp"
#include "helper/formats.hpp"
#include "jaythread/sync.hpp"
#include "sdkconfig.h"
#include "system_logger.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;

void ControlTask::reset() {
  hexa_controller.manual.reset();
  hexa_controller.automatic.reset();
  hexa_controller.semiautomatic.reset();
  hexa_controller.smart.reset();
}

void ControlTask::load_configs() {
  LOGI("loading control configs");
  hexa_controller.automatic.configs.load();
  hexa_controller.semiautomatic.configs.load();
  hexa_controller.smart.configs.load();
  LOGI("control configs loaded");
}

void ControlTask::init_configs() {
  LOGI("initializing control configs");
  hexa_controller.automatic.configs.init();
  hexa_controller.semiautomatic.configs.init();
  hexa_controller.smart.configs.init();
  LOGI("control configs initialized");
}

void ControlTask::main() {
  init_configs();
  load_configs();
  ControllerInput input;
  ControllerOutput output;
  auto freq = Period::from_hz(CONFIG_HEXA_TASKS_CONTROL_FREQ_HZ);
  input.freq = freq;
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
          output = hexa_controller.manual.run(input);
          break;
        case ControlMode::HEXA_AUTOMATIC:
          output = hexa_controller.automatic.run(input);
          break;
        case ControlMode::HEXA_SEMIAUTOMATIC:
          output = hexa_controller.semiautomatic.run(input);
          break;
        case ControlMode::HEXA_SMART:
          output = hexa_controller.smart.run(input);
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
    Sync::sleep(freq.ms());
  }
}