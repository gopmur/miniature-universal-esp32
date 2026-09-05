// #include "services/monitor.hpp"
// #include "esp_log.h"
// #include "esp_system.h"
// #include "freertos/idf_additions.h"
// #include "service.hpp"
// #include "thread.hpp"

// MonitorService::MonitorService(int priority) : Service(priority, "monitor") {}

// void MonitorService::update_task_list() {
//   TaskStatus_t runtime_status[32];
//   uint32_t total_runtime;
//   int task_count = uxTaskGetSystemState(runtime_status, 32, &total_runtime);
//   auto idle_handle_0 = xTaskGetIdleTaskHandleForCore(0);
//   auto idle_handle_1 = xTaskGetIdleTaskHandleForCore(1);
//   for (int i = 0; i < task_count; i++) {
//     bool exists = false;
//     auto task_status = runtime_status[i];
//     auto task_handle = task_status.xHandle;
//     for (auto task : task_list) {
//       if (task.get_handle() == task_handle) {
//         exists = true;
//         break;
//       }
//     }
//     if (!exists && task_handle != idle_handle_0 && task_handle != idle_handle_1) {
//       task_list.push_back(ThreadWrapper(task_handle));
//     }
//   }
//   for (int i = 0; i < task_list.size(); i++) {
//     bool exists = false;
//     for (int j = 0; j < task_count; j++) {
//       auto task_status = runtime_status[j];
//       auto task_handle = task_status.xHandle;
//       if (task_handle == task_list[i].get_handle()) {
//         exists = true;
//         break;
//       }
//     }
//     if (!exists) {
//       task_list.erase(task_list.begin() + i);
//     }
//   }
// }

// void MonitorService::main() {
//   while (true) {
//     task_list_mutex.take();
//     update_task_list();
//     for (auto& service : task_list) {
//       service.update_runtime_stats();
//     }
//     task_list_mutex.give();
//     vTaskDelay(250);
//   }
// }

// std::vector<ThreadWrapper> MonitorService::get_task_list() {
//   task_list_mutex.take();
//   auto task_list_copy = task_list;
//   task_list_mutex.give();
//   return task_list_copy;
// }