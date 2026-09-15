#pragma once

#include "controller.hpp"

struct AutomaticControlParamsLeg {
  float timeout = 200;
  float torque = 5;
  float velocity_threshold = 1.431169987;
};

struct AutomaticControlParams {
  AutomaticControlParamsLeg left;
  AutomaticControlParamsLeg right;
};

class AutomaticController : public Controller {
  private:
  float right_timer = 0;
  float left_timer = 0;
  bool ro = false;
  bool ri = false;
  bool lo = false;
  bool li = false;

  public:
  void reset();
  AutomaticControlParams params;
  ControllerOutput run(ControllerInput input);
};
