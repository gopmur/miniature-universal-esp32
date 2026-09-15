#include "controller/zero.hpp"
#include <cmath>

ControllerOutput ZeroController::run(ControllerInput input) {
  ControllerOutput output;
  output.left_motor.torque = 0;
  output.right_motor.torque = 0;
  return output;
}