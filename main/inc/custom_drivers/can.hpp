#pragma once

#include "custom_drivers/can/packet.hpp"
#include "esp_err.h"
#include "esp_twai_types.h"
#include "system_logger.hpp"

class Can {
  MAKE_LOGGABLE("can_driver");

  private:
  twai_node_handle_t twai;
  Mutex mutex;
  void restart();

  public:
  esp_err_t send_packet(CanPacket packet);
  void init();
};