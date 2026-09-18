#pragma once

#include "jaythread/thread.hpp"

class LoggerTask : public Thread {
  private:
  std::string tag = "logger";
  volatile bool is_logging = false;
  void main();

  public:
  void start_new_log();
  void stop_log();
};