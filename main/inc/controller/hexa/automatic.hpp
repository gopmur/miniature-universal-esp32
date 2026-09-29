#pragma once

#include "controller/hexa.hpp"

struct HexaAutomaticControlParamsLeg {
  float timeout = 200;
  float torque = 5;
  float velocity_threshold = 1.431169987;
};

struct HexaAutomaticControlParams {
  HexaAutomaticControlParamsLeg left;
  HexaAutomaticControlParamsLeg right;
};

class HexaAutomaticController : public HexaController {
  private:
  float right_timer = 0;
  float left_timer = 0;
  bool ro = false;
  bool ri = false;
  bool lo = false;
  bool li = false;
  HexaControllerOutput run(HexaControllerInput input);

  public:
  void reset();
  HexaAutomaticControlParams params;
  using HexaController::run;
};
