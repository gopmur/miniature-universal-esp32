#pragma once

#include <array>
#include <cstdint>
#include "hal/twai_types.h"

struct MotorPacket {
  twai_frame_header_t header;
  std::array<uint8_t, 8> data;
};
