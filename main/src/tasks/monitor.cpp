#include "tasks/monitor.hpp"
#include "jaythread/executable.hpp"
#include "jaythread/sync.hpp"


std::vector<ThreadStatus> MonitorTask::get_threads_status() {
  threads_status_mutex.take();
  auto threads_status = this->threads_status;
  threads_status_mutex.give();
  return threads_status;
}

void MonitorTask::main() {
  while (true) {
    threads_status_mutex.take();
    threads_status = Executable::get_threads_status();
    threads_status_mutex.give();
    Sync::sleep(1000);
  }
}
