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

double ControlTask::torque_profile(double count_timer, int total_time) {
  double a = 0.15 * total_time;
  double b = 0.70 * total_time;
  double c = 0.15 * total_time;
  double c1 = 10 / a;
  double c2 = 10 / c;

  if (count_timer <= a && count_timer >= 0) {
    return 1 / (1 + exp(-c1 * (count_timer - a / 2)));
  } else if (count_timer > a && count_timer < a + b) {
    return 1;
  } else if (count_timer >= a + b && count_timer <= a + b + c) {
    return 1 / (1 + exp(c2 * (count_timer - (a + b + c / 2))));
  } else {
    return 0;
  }
}

void ControlTask::main() {
  ControllerInput input;
  ControllerOuput output;
  while (true) {
    input.left_motor.position = left_motor->get_position();
    input.right_motor.position = right_motor->get_position();
    input.left_motor.velocity = left_motor->get_velocity();
    input.right_motor.velocity = right_motor->get_velocity();
    output = manual_controller.run(input);
    motor_task->set_torque(output.left_motor.torque, output.right_motor.torque);
    Sync::sleep(10);
  }
}