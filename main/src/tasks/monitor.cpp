#include "tasks/monitor.hpp"
#include "jaythread/executable.hpp"
#include "jaythread/sync.hpp"
#include "sdkconfig.h"
#include "tasks/ws.hpp"

extern WebSocketTask ws_task;

std::vector<ThreadStatus> MonitorTask::get_threads_status() {
  threads_status_mutex.take();
  auto threads_status = this->threads_status;
  threads_status_mutex.give();
  return threads_status;
}

void MonitorTask::main() {
  Period freq = Period::from_ms(CONFIG_HEXA_TASKS_MONITOR_FREQ_MS);
  while (true) {
    threads_status_mutex.take();
    threads_status = Executable::get_threads_status();
    threads_status_mutex.give();
    auto thread_status = get_threads_status();
    ws_task.send_to_task_connections(thread_status);
    Sync::sleep(freq.ms());
  }
}
