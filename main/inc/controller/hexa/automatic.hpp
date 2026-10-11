#pragma once

#include "controller/hexa.hpp"

struct HexaAutomaticControlParamsLeg {
  float torque = 5;
};

struct HexaAutomaticControlParams {
  HexaAutomaticControlParamsLeg left;
  HexaAutomaticControlParamsLeg right;
};

class HexaAutomaticControlConfigs {
  private:
  static constexpr const char* nvs_namespace = "hexa_automatic";
  static constexpr const char* position_difference_key = "pos_diff";
  static constexpr const char* left_velocity_threshold_key = "left_vel_th";
  static constexpr const char* right_velocity_threshold_key = "right_vel_th";
  static constexpr const char* right_timeout_key = "right_timeout";
  static constexpr const char* left_timeout_key = "left_timeout";

  float position_difference = 0.286483699;
  float left_velocity_threshold = 1.431169987;
  float right_velocity_threshold = 1.431169987;
  int32_t left_timeout = 200;
  int32_t right_timeout = 200;

  public:
  void set_position_difference(float position_difference);
  void set_left_velocity_threshold(float left_velocity_threshold);
  void set_right_velocity_threshold(float right_velocity_threshold);
  void set_left_timeout(int32_t left_timeout);
  void set_right_timeout(int32_t right_timeout);
  float get_position_difference();
  float get_left_velocity_threshold();
  float get_right_velocity_threshold();
  int32_t get_left_timeout();
  int32_t get_right_timeout();
  void load();
  void init();
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
  HexaAutomaticControlConfigs configs;
  HexaAutomaticControlParams params;
  using HexaController::run;
};
