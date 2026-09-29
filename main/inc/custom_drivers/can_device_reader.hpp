#pragma once

#include "custom_drivers/can/packet.hpp"

class AbstractCanDeviceReader {
  public:
  virtual void consume(CanPacket packet) = 0;
};
