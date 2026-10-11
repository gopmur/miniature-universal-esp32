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

struct HexaControllerGroup {
  HexaManualController manual;
  HexaAutomaticController automatic;
  HexaSemiautomaticController semiautomatic;
  HexaSmartController smart;
};

class ControlTask : public Thread {
  MAKE_LOGGABLE("control_task");

  public:
  bool running = false;
  ControlMode control_mode = ControlMode::HEXA_MANUAL;
  ControlMode prev_control_mod = ControlMode::HEXA_MANUAL;
  ZeroController zero_controller;
  HexaControllerGroup hexa_controller;

  private:
  void init_configs();
  void load_configs();
  void reset();
  void main();
};