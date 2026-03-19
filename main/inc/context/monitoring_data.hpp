#pragma once

#include <cstdint>
struct MonitoringData {
  float dns_service_cpu_usage;
  float led_service_cpu_usage;
  float monitor_service_cpu_usage;
  float stm_uart_rx_service_cpu_usage;
  float ws_service_cpu_usage;

  uint32_t dns_service_stack_usage;
  uint32_t led_service_stack_usage;
  uint32_t monitor_service_stack_usage;
  uint32_t stm_uart_rx_service_stack_usage;
  uint32_t ws_service_stack_usage;
};

extern MonitoringData monitoring_data;
