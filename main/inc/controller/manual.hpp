#pragma once

#include "controller.hpp"


struct ManualControlParamsLeg {
  float torque = 0;
};

struct ManualControlParams {
  ManualControlParamsLeg left;
  ManualControlParamsLeg right;
};

class ManualController : public Controller {
  public:
  ManualControlParams params;
  ControllerOutput run(ControllerInput input);
};