#pragma once

#include <cstdint>
#include "controller/automatic.hpp"
#include "controller/manual.hpp"
#include "controller/semi_automatic.hpp"
#include "controller/smart.hpp"
#include "controller/zero.hpp"
#include "jaythread/thread.hpp"
#include "loggable.hpp"

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

class ControlTask : public Thread {
  MAKE_LOGGABLE("control_task");

  public:
  bool running = false;
  ControlMode control_mode = ControlMode::MANUAL;
  ControlMode prev_control_mod = ControlMode::MANUAL;
  ZeroController zero_controller;
  ManualController manual_controller;
  AutomaticController automatic_controller;
  SemiautomaticController semiautomatic_controller;
  SmartController smart_controller;

  private:
  void reset();
  void main();
};