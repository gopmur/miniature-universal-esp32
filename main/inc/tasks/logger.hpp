#pragma once

#include "jaythread/thread.hpp"
#include "system_logger.hpp"

class LoggerTask : public Thread {
  MAKE_LOGGABLE("logger_task");

  private:
  volatile bool is_logging = false;
  FILE* log_file = nullptr; 
  void main();
  void write_header();
  void write_data();

  public:
  void start_new_log();
  void stop_log();
};