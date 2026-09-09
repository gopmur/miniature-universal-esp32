#pragma once

#include <array>
#include "hal/twai_types.h"

struct CanPacket {
  twai_frame_header_t header;
  std::array<uint8_t, 8> data;
};