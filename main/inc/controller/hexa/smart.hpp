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

class HexaSmartControlConfigs {
  private:
  static constexpr const char* nvs_namespace = "hexa_smart";
  static constexpr const char* velocity_threshold_key = "vel_th";
  static constexpr const char* right_timeout_key = "r_timeout";
  static constexpr const char* left_timeout_key = "l_timeout";
  static constexpr const char* right_delay_timeout_key = "r_delay";
  static constexpr const char* left_delay_timeout_key = "l_delay";

  float velocity_threshold = 0.8;
  uint32_t right_timeout = 300;
  uint32_t left_timeout = 300;
  uint32_t right_delay_timeout = 20;
  uint32_t left_delay_timeout = 20;

  public:
  float get_velocity_threshold();
  uint32_t get_right_timeout();
  uint32_t get_left_timeout();
  uint32_t get_right_delay_timeout();
  uint32_t get_left_delay_timeout();
  void set_velocity_threshold(float velocity_threshold);
  void set_right_timeout(uint32_t right_timeout);
  void set_left_timeout(uint32_t left_timeout);
  void set_right_delay_timeout(uint32_t right_delay_timeout);
  void set_left_delay_timeout(uint32_t left_delay_timeout);
  void init();
  void load();
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
  HexaSmartControlConfigs configs;
  void reset();
  using HexaController::run;
};