#pragma once

struct MonitoringData {
  float dns_service_cpu_usage;
  float led_service_cpu_usage;
  float monitor_service_cpu_usage;
  float stm_uart_rx_service_cpu_usage;
  float ws_service_cpu_usage;
};

extern MonitoringData monitoring_data;
