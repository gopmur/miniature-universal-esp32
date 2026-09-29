#include "tasks/can_recv.hpp"

void CanRecvTask::main() {
  while (true) {
    auto packet_result = packet_queue.receive();
    if (!packet_result.has_value()) {
      continue;
    }
    auto packet = packet_result.value();
    for (auto mask : masks) {
      uint32_t target_id = packet.header.id & mask;
      if (driver_map.contains(target_id)) {
        driver_map[target_id]->consume(packet);
      }
    }
  }
}

void CanRecvTask::bind(uint32_t id, AbstractCanDeviceReader* reader) {
  driver_map[id] = reader;
}

void CanRecvTask::bind(uint32_t id, uint32_t mask, AbstractCanDeviceReader* callback) {
  bool mask_exists = false;
  for (auto stored_mask : masks) {
    if (stored_mask == mask) {
      mask_exists = true;
      break;
    }
  }
  if (!mask_exists) {
    masks.push_back(mask);
  }
  bind(id, callback);
}
