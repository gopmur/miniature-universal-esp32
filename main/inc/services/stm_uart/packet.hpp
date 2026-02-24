#pragma once

#include <cstdint>

enum class LapplAddress : uint8_t {
  RUNNING,
  LEFT_TORQUE,
  RIGHT_TORQUE,
  CONTROL_MODE,
};

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};
