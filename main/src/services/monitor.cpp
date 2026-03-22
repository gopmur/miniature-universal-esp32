#include "services/monitor.hpp"
#include "freertos/idf_additions.h"
#include "service.hpp"

MonitorService::MonitorService(int priority) : Service(priority, "monitor") {}

void MonitorService::main() {
  while (true) {
    vTaskDelay(250);
    auto thread_list = Thread::get_thread_list();
    for (auto service : thread_list) {
      service->update_runtime_stats();
    }
  }
}