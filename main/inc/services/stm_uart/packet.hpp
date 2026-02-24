#pragma once

#include <cstdint>

enum class ControlMode : uint8_t {
  MANUAL,
  AUTO,
  SEMI_AUTO,
  SMART,
};

const char* get_contorl_mode_str(ControlMode mode);