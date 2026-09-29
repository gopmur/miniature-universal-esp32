#pragma once

#include <cstdint>
#include "controller/hexa/automatic.hpp"
#include "controller/hexa/manual.hpp"
#include "controller/hexa/semi_automatic.hpp"
#include "controller/hexa/smart.hpp"
#include "controller/zero.hpp"
#include "jaythread/thread.hpp"
#include "system_logger.hpp"

enum class ControlMode : uint8_t {
  HEXA_MANUAL,
  HEXA_AUTOMATIC,
  HEXA_SEMIAUTOMATIC,
  HEXA_SMART,
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
  ControlMode control_mode = ControlMode::HEXA_MANUAL;
  ControlMode prev_control_mod = ControlMode::HEXA_MANUAL;
  ZeroController zero_controller;
  HexaManualController hexa_manual_controller;
  HexaAutomaticController hexa_automatic_controller;
  HexaSemiautomaticController hexa_semiautomatic_controller;
  HexaSmartController hexa_smart_controller;

  private:
  void reset();
  void main();
};