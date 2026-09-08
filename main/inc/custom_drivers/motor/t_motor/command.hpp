#pragma once

#include <cstdint>

enum class TMotorCommand : uint8_t {
  DISABLE = 0xfd,
  ENABLE = 0xfc,
  ZERO_POS = 0xfe,
};
