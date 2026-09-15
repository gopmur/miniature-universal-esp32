#include "controller/automatic.hpp"
#include "controller.hpp"
#include "esp_log.h"

ControllerOutput AutomaticController::run(ControllerInput input) {
  ControllerOutput output;
  float torque_profile_right_value = 0;
  float torque_profile_left_value = 0;
  if (right_timer > params.right.timeout) {
    ro = false;
  } else if (input.right_motor.velocity > params.right.velocity_threshold &&
             (input.right_motor.position - input.left_motor.position) < -0.286483699 &&
             input.left_motor.velocity <= 0 && ri == false) {
    ro = true;
    ri = true;
    li = false;
  }

  if (left_timer > params.left.timeout) {
    lo = false;
  } else if (input.left_motor.velocity > params.left.velocity_threshold &&
             (input.right_motor.position - input.left_motor.position) > 0.286483699 &&
             input.right_motor.velocity < 0.0 && li == false) {
    lo = true;
    ri = false;
    li = true;
  }

  if (ro == true) {
    right_timer = right_timer + 1;
    torque_profile_right_value = torque_profile(right_timer, params.right.timeout);
  } else if (ro == false) {
    right_timer = 0;
    torque_profile_right_value = 0;
  }

  if (lo == true) {
    left_timer = left_timer + 1;
    torque_profile_left_value = torque_profile(left_timer, params.left.timeout);
  } else if (lo == false) {
    left_timer = 0;
    torque_profile_left_value = 0;
  }

  output.left_motor.torque = params.left.torque * torque_profile_left_value;
  output.right_motor.torque = params.right.torque * torque_profile_right_value;

  return output;
}

void AutomaticController::reset() {
  right_timer = 0;
  left_timer = 0;
  ro = false;
  ri = false;
  lo = false;
  li = false;
}
