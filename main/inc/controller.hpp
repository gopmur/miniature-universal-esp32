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

struct ControllerOutput {
  ControllerMotorOutput left_motor;
  ControllerMotorOutput right_motor;
};

class Controller {
  public:
  float torque_profile(float count_timer, int total_time);
  virtual void reset();
  virtual ControllerOutput run(ControllerInput input) = 0;
};