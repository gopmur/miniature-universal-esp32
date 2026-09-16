#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "custom_drivers/can_device_reader.hpp"
#include "esp_twai_types.h"
#include "hal/twai_types.h"
#include "jaythread/ipc/queue.hpp"
#include "jaythread/thread.hpp"

class CanRecvTask : public Thread {
  private:
  std::unordered_map<uint32_t, AbstractCanDeviceReader*> driver_map;
  std::vector<uint32_t> masks = {UINT32_MAX};
  void main();

  public:
  void bind(uint32_t id, AbstractCanDeviceReader* reader);
  void bind(uint32_t mask, uint32_t id, AbstractCanDeviceReader* reader);
  Queue<CanPacket, 8> packet_queue;
};
