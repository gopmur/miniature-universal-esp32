#pragma once

#include <vector>
#include "controller/hexa.hpp"

struct HexaSmartControlParamsLeg {
  float torque = 5;
};

struct HexaSmartControlParams {
  HexaSmartControlParamsLeg left;
  HexaSmartControlParamsLeg right;
};

class HexaSmartController : public HexaController {
  private:
  int right_delay_timer = 200;
  int left_delay_timer = 200;
  float previous_left_state = 0;
  float previous_right_state = 0;
  float fuzzy_timer_r = 1000;
  float fuzzy_timer_l = 1000;
  float right_state = 0;
  float left_state = 0;
  bool right_activated = false;
  bool left_activated = false;
  float calculate_output(const std::vector<float>& x);
  HexaControllerOutput run(HexaControllerInput input);

  public:
  HexaSmartControlParams params;
  void reset();
  using HexaController::run;
};