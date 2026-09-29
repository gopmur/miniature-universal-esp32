#include "controller/zero.hpp"
#include <cmath>

ControllerOutput ZeroController::run(ControllerInput input) {
  ControllerOutput output;
  output.torques.fill(0);
  return output;
}