#pragma once

#include "esp_event_base.h"
#include "system_logger.hpp"

class WifiEventCallback {
  MAKE_LOGGABLE("wifi");

  public:
  static void wifi_event_handler(void* arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data);
};
