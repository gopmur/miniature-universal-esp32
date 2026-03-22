#include "services/monitor.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "service.hpp"
#include "thread.hpp"

MonitorService::MonitorService(int priority) : Service(priority, "monitor") {}

void MonitorService::main() {
  while (true) {
    TaskStatus_t runtime_status[32];
    uint32_t total_runtime;
    int task_count = uxTaskGetSystemState(runtime_status, 32, &total_runtime);
    for (int i = 0; i < task_count; i++) {
      auto task_status = runtime_status[i];
      auto task_handle = task_status.xHandle;
      auto idle_task_handle_0 = xTaskGetIdleTaskHandleForCore(0);
      auto idle_task_handle_1 = xTaskGetIdleTaskHandleForCore(1);
      if (task_handle == idle_task_handle_0 || task_handle == idle_task_handle_1) {
        continue;
      }
      ThreadWrapper wrapped_task(task_handle);
      wrapped_task.register_to_list();
    }
    auto thread_list = Thread::get_thread_list();
    ESP_LOGI("MON", "Heap Size %d", esp_get_free_heap_size());
    for (auto service : thread_list) {
      service->update_runtime_stats();
    }
    vTaskDelay(250);
  }
}