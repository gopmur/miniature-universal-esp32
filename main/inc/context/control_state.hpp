#pragma once

#include <cstdint>

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

struct AutomaticControlParamsLeg {
  float timeout = 200;
  float torque = 5;
  float velocity_threshold = 1.431169987;
};

struct AutomaticControlParams {
  AutomaticControlParamsLeg left;
  AutomaticControlParamsLeg right;
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

struct ManualControlParamsLeg {
  float torque = 0;
};

struct ManualControlParams {
  ManualControlParamsLeg left;
  ManualControlParamsLeg right;
};

struct ControlParams {
  ManualControlParams manual;
  AutomaticControlParams automatic;
  SemiautomaticControlParams semiautomatic;
  SmartControlParams smart;
};

struct ControlState {
  bool running = false;
  ControlMode control_mode = ControlMode::MANUAL;
  ControlParams control_params;
};

extern ControlState control_state;