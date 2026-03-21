#include "services/monitor.hpp"
#include "freertos/idf_additions.h"
#include "service.hpp"

MonitorService::MonitorService(int priority) : Service(priority, "monitor") {}

void MonitorService::main() {
  while (true) {
    vTaskDelay(250);
    for (auto service : ServiceThread::service_list) {
      service->update_runtime_stats();
    }
  }
}