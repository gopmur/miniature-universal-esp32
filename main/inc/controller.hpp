#pragma once

struct ControllerMotorInput {
  float velocity;
  float position;
};

struct ControllerInput {
  ControllerMotorInput left_motor;
  ControllerMotorInput right_motor;
};

struct ControllerMotorOutput {
  float torque;
};

struct ControllerOuput {
  ControllerMotorOutput left_motor;
  ControllerMotorOutput right_motor;
};

class Controller {
  public:
  virtual ControllerOuput run(ControllerInput input);
};