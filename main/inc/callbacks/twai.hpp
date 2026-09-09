#pragma once

#include "esp_twai_types.h"

class TwaiCallback {

  public:
  static bool rx_done(twai_node_handle_t handle,
                      const twai_rx_done_event_data_t* edata,
                      void* user_ctx);
};