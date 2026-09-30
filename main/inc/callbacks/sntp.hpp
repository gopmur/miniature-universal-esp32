#pragma once

#include "system_logger.hpp"

class SntpCallback {
  MAKE_LOGGABLE("sntp_callback");

  public:
  static void sync_done(struct timeval* tv);
};