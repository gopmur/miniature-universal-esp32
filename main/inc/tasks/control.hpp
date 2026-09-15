#pragma once

#include <cstdint>
#include "controller/automatic.hpp"
#include "controller/manual.hpp"
#include "jaythread/thread.hpp"

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};

struct Data3D {
  float x;
  float y;
  float z;
};

enum class Leg {
  LEFT,
  RIGHT,
};

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

struct SmartControlParamsLeg {
  float torque = 5;
};

struct SmartControlParams {
  SmartControlParamsLeg left;
  SmartControlParamsLeg right;
};

struct ControlParams {
  SemiautomaticControlParams semiautomatic;
  SmartControlParams smart;
};

class ControlTask : public Thread {
  public:
  bool running = false;
  ControlMode control_mode = ControlMode::MANUAL;
  ControlParams control_params;
  ManualController manual_controller;
  AutomaticController automatic_controller;

  private:
  std::string tag = "control task";
  void main();
};