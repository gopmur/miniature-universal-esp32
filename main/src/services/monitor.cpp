#include "services/monitor.hpp"
#include "freertos/idf_additions.h"
#include "service.hpp"

#include "context/monitoring_data.hpp"
#include "context/services/dns.hpp"
#include "context/services/led.hpp"
#include "context/services/stm_uart_rx.hpp"
#include "context/services/ws.hpp"

MonitorService::MonitorService(int priority)
    : AbstractService(priority, "monitor") {}

void MonitorService::main() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-cxx-extension"
  TaskStatus_t task_status[32];
#pragma clang diagnostic pop
  uint32_t total_runtime;
  while (true) {
    int task_count = uxTaskGetNumberOfTasks();
    vTaskDelay(250);
    uxTaskGetSystemState(task_status, 32, &total_runtime);
    for (int i = 0; i < task_count; i++) {
      auto min_free_stack = task_status[i].usStackHighWaterMark;
      auto task_handle = task_status[i].xHandle;
      auto runtime = task_status[i].ulRunTimeCounter;
      if (task_handle == dns_service.get_handle()) {
        auto stack_usage = dns_service.stack_size - min_free_stack;
        monitoring_data.dns_service_stack_usage = stack_usage;
        monitoring_data.dns_service_cpu_usage =
            dns_service.calculate_cpu_usage(runtime, total_runtime);
      } else if (task_handle == stm_uart_rx_service.get_handle()) {
        auto stack_usage = stm_uart_rx_service.stack_size - min_free_stack;
        monitoring_data.stm_uart_rx_service_stack_usage = stack_usage;
        monitoring_data.stm_uart_rx_service_cpu_usage =
            stm_uart_rx_service.calculate_cpu_usage(runtime, total_runtime);
      } else if (task_handle == led_service.get_handle()) {
        auto stack_usage = led_service.stack_size - min_free_stack;
        monitoring_data.led_service_stack_usage = stack_usage;
        monitoring_data.led_service_cpu_usage =
            led_service.calculate_cpu_usage(runtime, total_runtime);
      } else if (task_handle == ws_service.get_handle()) {
        auto stack_usage = ws_service.stack_size - min_free_stack;
        monitoring_data.ws_service_stack_usage = stack_usage;
        monitoring_data.ws_service_cpu_usage =
            ws_service.calculate_cpu_usage(runtime, total_runtime);
      } else if (task_handle == this->handle) {
        auto stack_usage = this->stack_size - min_free_stack;
        monitoring_data.monitor_service_stack_usage = stack_usage;
        monitoring_data.monitor_service_cpu_usage =
            this->calculate_cpu_usage(runtime, total_runtime);
      }
    }
  }
}