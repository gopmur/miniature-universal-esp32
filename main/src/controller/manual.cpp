#include "controller/manual.hpp"
#include <cmath>

ControllerOutput ManualController::run(ControllerInput input) {
  ControllerOutput output;
  output.left_motor.torque = params.left.torque;
  output.right_motor.torque = params.right.torque;
  return output;
}