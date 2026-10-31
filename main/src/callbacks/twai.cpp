#include "callbacks/twai.hpp"
#include <cstdint>
#include <cstring>
#include "esp_twai.h"
#include "system_logger.hpp"
#include "tasks/can_recv.hpp"

extern CanRecvTask can_recv_task;

bool TwaiCallback::rx_done(twai_node_handle_t handle,
                           const twai_rx_done_event_data_t* edata,
                           void* user_ctx) {
  uint8_t recv_buff[64];
  twai_frame_t rx_frame = {
      .buffer = recv_buff,
      .buffer_len = sizeof(recv_buff),
  };

  if (twai_node_receive_from_isr(handle, &rx_frame) == ESP_OK) {
    CanPacket packet;
    packet.header = rx_frame.header;
    memcpy(packet.data.data(), rx_frame.buffer, rx_frame.buffer_len);
    if (!can_recv_task.packet_queue.send_from_isr(packet)) {
      LOGE("received packet 0x%02x but dropped due to receive queue being full", packet.header.id);
    }
  }
  return false;
}