#pragma once

#include <array>
#include "custom_drivers/motor.hpp"
#include "sdkconfig.h"
enum class Leg {
  LEFT,
  RIGHT,
};


struct ControllerMotorInput {
  float velocity;
  float position;
};

struct ControllerInput {
  std::array<MotorFeedback, CONFIG_HEXA_MOTOR_COUNT> motor_feedbacks;
};

struct ControllerMotorOutput {
  float torque;
};

struct ControllerOutput {
  std::array<float, CONFIG_HEXA_MOTOR_COUNT> torques;
};

class Controller {
  public:
  float torque_profile(float count_timer, int total_time);
  virtual void reset();
  virtual ControllerOutput run(ControllerInput input) = 0;
};