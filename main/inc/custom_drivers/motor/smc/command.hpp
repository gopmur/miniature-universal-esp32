#pragma once

#include <cstdint>

enum class SmcMotorCommand : uint8_t {
  DISABLE = 0x80,
  ENABLE = 0x88,
  ZERO_POS = 0x95,
  READ_ENCODER = 0x90,
  TORQUE = 0xa1,
};
