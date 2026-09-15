#pragma once

#include <vector>
#include "controller.hpp"

struct SmartControlParamsLeg {
  float torque = 5;
};

struct SmartControlParams {
  SmartControlParamsLeg left;
  SmartControlParamsLeg right;
};

class SmartController : public Controller {
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

  public:
  SmartControlParams params;
  ControllerOutput run(ControllerInput input);
  void reset();
};