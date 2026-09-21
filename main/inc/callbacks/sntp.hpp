#pragma once

#include "tasks/wifi_con_handler.hpp"

class SntpCallback {
  MAKE_LOGGABLE("sntp_callback");

  public:
  static void sync_done(struct timeval* tv);
};