#include "context/monitoring_data.hpp"

MonitoringData monitoring_data = {
    .dns_service_cpu_usage = 0,
    .led_service_cpu_usage = 0,
    .monitor_service_cpu_usage = 0,
    .stm_uart_rx_service_cpu_usage = 0,
    .ws_service_cpu_usage = 0,
};