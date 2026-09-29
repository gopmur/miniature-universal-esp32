#include "controller/hexa/manual.hpp"
#include <cmath>

HexaControllerOutput HexaManualController::run(HexaControllerInput input) {
  HexaControllerOutput output;
  output.left_motor.torque = params.left.torque;
  output.right_motor.torque = params.right.torque;
  return output;
}
