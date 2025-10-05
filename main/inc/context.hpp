#pragma once

#include "freertos/idf_additions.h"
#include "services/dns.hpp"
#include "services/http.hpp"
#include "services/led.hpp"
#include "services/stm_uart.hpp"

namespace context {

void init();

extern HttpService http_service;
extern DnsService dns_service;
extern LedService led_service;
// extern StmUartService stm_uart_service;

// extern QueueHandle_t http_to_stm_uart_queue;
// extern QueueHandle_t stm_uart_to_http_queue;
}  // namespace context