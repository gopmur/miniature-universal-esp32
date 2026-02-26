#include "context.hpp"
#include "services/led.hpp"
#include "services/stm_uart_rx.hpp"
#include "services/ws.hpp"

namespace context {

  HttpService http_service;
  DnsService dns_service("192.168.4.1");
  LedService led_service;

  StmUartRxService stm_uart_rx_service(config::stm_uart::port);
  WebSocketService ws_service;
}