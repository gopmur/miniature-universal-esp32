#pragma once

#include <cstdint>
#include "controller/automatic.hpp"
#include "controller/manual.hpp"
#include "controller/semi_automatic.hpp"
#include "controller/zero.hpp"
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

struct SmartControlParamsLeg {
  float torque = 5;
};

struct SmartControlParams {
  SmartControlParamsLeg left;
  SmartControlParamsLeg right;
};

struct ControlParams {
  SmartControlParams smart;
};

class ControlTask : public Thread {
  public:
  bool running = false;
  ControlMode control_mode = ControlMode::MANUAL;
  ControlMode prev_control_mod = ControlMode::MANUAL;
  ControlParams control_params;
  ZeroController zero_controller;
  ManualController manual_controller;
  AutomaticController automatic_controller;
  SemiautomaticController semiautomatic_controller;

  private:
  std::string tag = "control task";
  void reset();
  void main();
};