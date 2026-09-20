#pragma once

#include "jaythread/thread.hpp"
#include "loggable.hpp"

class LoggerTask : public Thread {
  MAKE_LOGGABLE("logger_task");

  private:
  volatile bool is_logging = false;
  void main();

  public:
  void start_new_log();
  void stop_log();
};