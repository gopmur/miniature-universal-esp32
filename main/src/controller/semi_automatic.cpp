#include "controller/semi_automatic.hpp"
#include <cstdlib>
#include "controller.hpp"

ControllerOutput SemiautomaticController::run(ControllerInput input) {
  ControllerOutput output;
  output.left_motor.torque = 0;
  output.right_motor.torque = 0;
  if (params.weak_leg == Leg::RIGHT) {
    if ((input.left_motor.position - input.right_motor.position) > params.start_assist_angle &&
        std::abs(input.left_motor.velocity) < epsilon &&
        std::abs(input.right_motor.velocity) < epsilon) {
      activate_assistance = true;
    }
    if (activate_assistance) {
      if (++d_t >= params.right.delay) {
        r_t++;
        output.right_motor.torque = params.right.torque;
      }
      if ((input.left_motor.position - input.right_motor.position) < -params.stop_assist_angle ||
          r_t > params.right.timeout) {
        activate_assistance = false;
        r_t = 0;
        d_t = 0;
        output.right_motor.torque = 0;
      }
    }
    output.left_motor.torque = 0;
  } else {
    if ((input.right_motor.position - input.left_motor.position) > params.start_assist_angle &&
        std::abs(input.left_motor.velocity) < epsilon &&
        std::abs(input.right_motor.velocity) < epsilon) {
      activate_assistance = true;
    }
    if (activate_assistance) {
      if (++d_t >= params.left.delay) {
        l_t++;
        output.left_motor.torque = params.left.torque;
      }
      if ((input.right_motor.position - input.left_motor.position) < -params.stop_assist_angle ||
          l_t > params.left.timeout) {
        activate_assistance = false;
        l_t = 0;
        d_t = 0;
        output.left_motor.torque = 0;
      }
    }
    output.right_motor.torque = 0;
  }
  return output;
}

void SemiautomaticController::reset() {
  epsilon = 0.02864837;
  d_t = 0;
  r_t = 0;
  l_t = 0;
  activate_assistance = false;
}
