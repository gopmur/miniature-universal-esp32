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

void ControlTask::main() {
  ControllerInput input;
  ControllerOutput output;
  while (true) {
    ESP_LOGI("control",
             "left_vel: %8.3f, left_pos: %8.3f, right_vel: %8.3f, right_pos: %8.3f",
             input.left_motor.velocity,
             input.left_motor.position,
             input.right_motor.velocity,
             input.right_motor.position);
    input.left_motor.position = left_motor->get_position();
    input.right_motor.position = right_motor->get_position();
    input.left_motor.velocity = left_motor->get_velocity();
    input.right_motor.velocity = right_motor->get_velocity();
    output = automatic_controller.run(input);
    motor_task->set_torque(output.left_motor.torque, output.right_motor.torque);
    Sync::sleep(10);
  }
}