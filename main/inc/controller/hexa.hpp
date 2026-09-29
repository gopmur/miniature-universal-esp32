#pragma once

#include "controller.hpp"

#define HEXA_LEFT_MOTOR_INDEX 0
#define HEXA_RIGHT_MOTOR_INDEX 1

struct HexaControllerMotorInput {
  float position;
  float velocity;
};

struct HexaControllerInput {
  HexaControllerMotorInput left_motor;
  HexaControllerMotorInput right_motor;
};

struct HexaControllerMotorOutput {
  float torque;
};

struct HexaControllerOutput {
  HexaControllerMotorOutput left_motor;
  HexaControllerMotorOutput right_motor;
};

class HexaController : public Controller {
  protected:
  HexaControllerOutput virtual run(HexaControllerInput) = 0;
  public:
  ControllerOutput run(ControllerInput input);
};