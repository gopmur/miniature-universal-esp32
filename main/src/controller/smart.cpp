#include "controller/smart.hpp"
#include "controller.hpp"

float SmartController::calculate_output(const std::vector<float>& x) {
  float velocity = x[1];
  float last = x[2];

  if (last == 0) {
    if (velocity > 0.8) {
      return 1.0;
    } else {
      return 0.0;
    }
  } else {  // last == 1
    if (velocity > 0.8) {
      return 1.0;
    } else if (velocity >= 0.8) {
      return 1.0;
    } else {
      return 0.0;
    }
  }
}

ControllerOutput SmartController::run(ControllerInput input) {
  ControllerOutput output;
  float r_delta_angle = input.right_motor.position - input.left_motor.position;
  float l_delta_angle = input.left_motor.position - input.right_motor.position;

  if ((float)calculate_output({r_delta_angle, input.right_motor.velocity, previous_right_state}) !=
          previous_right_state &&
      fuzzy_timer_r > 300) {
    fuzzy_timer_r = 0;
    right_state =
        (float)calculate_output({r_delta_angle, input.right_motor.velocity, previous_right_state});
  } else {
    right_state = previous_right_state;
  }
  right_activated = (right_state > 0);

  if ((float)calculate_output({l_delta_angle, input.left_motor.velocity, previous_left_state}) !=
          previous_left_state &&
      fuzzy_timer_l > 300) {
    fuzzy_timer_l = 0;
    left_state =
        (float)calculate_output({l_delta_angle, input.left_motor.velocity, previous_left_state});
  } else {
    left_state = previous_left_state;
  }
  if (right_state == 0 && previous_right_state != 0) {
    left_delay_timer = 0;
  }
  if ((right_activated && params.right.torque != 0) || left_delay_timer < 20) {
    left_state = 0;
    if (left_state != previous_left_state) {
      fuzzy_timer_l = 0;
    }
  }
  if (left_state == 0 && previous_left_state != 0) {
    right_delay_timer = 0;
  }
  left_activated = (left_state > 0);
  if ((left_activated && params.left.torque != 0) || right_delay_timer < 20) {
    right_state = 0;
    if (right_state != previous_right_state) {
      fuzzy_timer_r = 0;
    }
  }
  previous_left_state = left_state;
  previous_right_state = right_state;
  fuzzy_timer_r++;
  fuzzy_timer_l++;
  right_delay_timer++;
  left_delay_timer++;
  float torque_profile_right_value = torque_profile(fuzzy_timer_r, 50);
  float torque_profile_left_value = torque_profile(fuzzy_timer_l, 50);
  output.right_motor.torque = params.right.torque * torque_profile_right_value * right_state;
  output.left_motor.torque = params.left.torque * torque_profile_left_value * left_state;
  return output;
}

void SmartController::reset() {
  previous_left_state = 0;
  previous_right_state = 0;
  fuzzy_timer_r = 1000;
  fuzzy_timer_l = 1000;
  right_state = 0;
  left_state = 0;
  right_activated = false;
  left_activated = false;
}
