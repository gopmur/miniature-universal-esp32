#pragma once

#include <vector>
#include "config.hpp"
#include "jaythread/executable.hpp"
#include "jaythread/ipc/mutex.hpp"
#include "jaythread/thread.hpp"

class MonitorTask : public Thread {
  private:
  std::vector<ThreadStatus> threads_status;
  Mutex threads_status_mutex;
  void main();

  public:
  std::vector<ThreadStatus> get_threads_status();
};