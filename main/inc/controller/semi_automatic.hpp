#pragma once

#include "controller.hpp"

struct SemiautomaticControlParamsLeg {
  float torque = 5;
  float delay = 1;
  float timeout = 100;
};

struct SemiautomaticControlParams {
  Leg weak_leg = Leg::LEFT;
  float start_assist_angle = 0.287979327;
  float stop_assist_angle = 2.583087293;
  SemiautomaticControlParamsLeg left;
  SemiautomaticControlParamsLeg right;
};

class SemiautomaticController : public Controller {
  private:
  float epsilon = 0.02864837;
  int d_t = 0;
  int r_t = 0;
  int l_t = 0;
  bool activate_assistance = false;

  public:
  SemiautomaticControlParams params;
  ControllerOutput run(ControllerInput input);
  void reset();
};
