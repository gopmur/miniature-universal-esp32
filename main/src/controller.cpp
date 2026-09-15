
#include "controller.hpp"
#include <cmath>

float Controller::torque_profile(float count_timer, int total_time) {
  float a = 0.15 * total_time;
  float b = 0.70 * total_time;
  float c = 0.15 * total_time;
  float c1 = 10 / a;
  float c2 = 10 / c;

  if (count_timer <= a && count_timer >= 0) {
    return 1 / (1 + exp(-c1 * (count_timer - a / 2)));
  } else if (count_timer > a && count_timer < a + b) {
    return 1;
  } else if (count_timer >= a + b && count_timer <= a + b + c) {
    return 1 / (1 + exp(c2 * (count_timer - (a + b + c / 2))));
  } else {
    return 0;
  }
}