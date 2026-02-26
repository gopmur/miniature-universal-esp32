#pragma once

#include "freertos/idf_additions.h"
#include "services/dns.hpp"
#include "services/http.hpp"
#include "services/led.hpp"
#include "services/stm_uart_rx.hpp"
#include "services/ws.hpp"

template <typename T>
struct DistantState {
  T value;
  bool pending;
};


namespace context {

  void init();

  extern HttpService http_service;
  extern DnsService dns_service;
  extern LedService led_service;
  extern StmUartRxService stm_uart_rx_service;
  extern WebSocketService ws_service;

  // extern QueueHandle_t http_to_stm_uart_queue;
  // extern QueueHandle_t stm_uart_to_http_queue;
}  // namespace context