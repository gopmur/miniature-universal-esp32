#include "tasks/can_recv.hpp"
#include "esp_log.h"
#include "esp_twai_types.h"
#include "jaythread/sync.hpp"

void CanRecvTask::main() {
  while (true) {
    auto packet_result = packet_queue.receive();
    if (!packet_result.has_value()) {
      continue;
    }
    auto packet = packet_result.value();
    twai_frame_t twai_frame = {
        .header = packet.header,
        .buffer = packet.data.data(),
        .buffer_len = packet.data.size(),
    };

    for (auto mask : masks) {
      uint32_t target_id = packet.header.id & mask;
      if (driver_map.contains(target_id)) {
        driver_map[target_id]->consume(packet);
        // ESP_LOGI("callback", "id %d", packet.header.id);
      }
    }
  }
}

void CanRecvTask::bind(uint32_t id, AbstractCanDeviceReader* reader) {
  driver_map[id] = reader;
}

void CanRecvTask::bind(uint32_t id, uint32_t mask, AbstractCanDeviceReader* callback) {
  masks.push_back(mask);
  bind(id, callback);
}
