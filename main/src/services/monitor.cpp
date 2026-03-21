#include "services/monitor.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "service.hpp"

MonitorService::MonitorService(int priority) : Service(priority, "monitor") {}

void MonitorService::main() {
  while (true) {
    vTaskDelay(100);
    ESP_LOGI("HEAP", "%d", esp_get_free_heap_size());
    ESP_LOGI("MIN HEAP", "%d", esp_get_minimum_free_heap_size());
    for (auto service : ServiceThread::service_list) {
      service->update_runtime_stats();
    }
  }
}