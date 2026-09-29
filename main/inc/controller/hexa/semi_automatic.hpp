#pragma once

#include "controller.hpp"
#include "controller/hexa.hpp"

struct HexaSemiautomaticControlParamsLeg {
  float torque = 5;
  float delay = 1;
  float timeout = 100;
};

struct HexaSemiautomaticControlParams {
  Leg weak_leg = Leg::LEFT;
  float start_assist_angle = 0.287979327;
  float stop_assist_angle = 2.583087293;
  HexaSemiautomaticControlParamsLeg left;
  HexaSemiautomaticControlParamsLeg right;
};

class HexaSemiautomaticController : public HexaController {
  private:
  float epsilon = 0.02864837;
  int d_t = 0;
  int r_t = 0;
  int l_t = 0;
  bool activate_assistance = false;
  HexaControllerOutput run(HexaControllerInput input);

  public:
  HexaSemiautomaticControlParams params;
  void reset();
  using HexaController::run;
};
