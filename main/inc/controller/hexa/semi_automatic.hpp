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
  HexaSemiautomaticControlParamsLeg left;
  HexaSemiautomaticControlParamsLeg right;
};

class HexaSemiautomaticControlConfigs {
  private:
  static constexpr const char* nvs_namespace = "hexa_semi_auto";
  static constexpr const char* start_assist_angle_key = "start_angle";
  static constexpr const char* stop_assist_angle_key = "stop_angle";
  static constexpr const char* epsilon_key = "epsilon";

  float start_assist_angle = 0.287979327;
  float stop_assist_angle = 2.583087293;
  float epsilon = 0.02864837;

  public:
  float get_start_assist_angle();
  float get_stop_assist_angle();
  float get_epsilon();
  void set_start_assist_angle(float start_assist_angle);
  void set_stop_assist_angle(float stop_assist_angle);
  void set_epsilon(float epsilon);
  void load();
  void init();
};

class HexaSemiautomaticController : public HexaController {
  private:
  int d_t = 0;
  int r_t = 0;
  int l_t = 0;
  bool activate_assistance = false;
  HexaControllerOutput run(HexaControllerInput input);

  public:
  HexaSemiautomaticControlConfigs configs;
  HexaSemiautomaticControlParams params;
  void reset();
  using HexaController::run;
};
