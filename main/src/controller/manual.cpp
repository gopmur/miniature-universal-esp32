#include "controller/manual.hpp"

ControllerOuput ManualController::run(ControllerInput input) {
  ControllerOuput output;
  output.left_motor.torque = params.left.torque;
  output.right_motor.torque = params.right.torque;
  return output;
}