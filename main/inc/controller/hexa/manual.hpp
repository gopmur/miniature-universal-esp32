#pragma once

#include "controller/hexa.hpp"


struct HexaManualControlParamsLeg {
  float torque = 0;
};

struct HexaManualControlParams {
  HexaManualControlParamsLeg left;
  HexaManualControlParamsLeg right;
};

class HexaManualController : public HexaController {
  private:
  HexaControllerOutput run(HexaControllerInput input);
  public:
  using HexaController::run;
  HexaManualControlParams params;
};