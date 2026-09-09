#pragma once

#include "esp_twai_types.h"
#include "custom_drivers/can_device_reader/can_packet.hpp"

class AbstractCanDeviceReader {
  public:
  virtual void consume(CanPacket packet) = 0;
};
