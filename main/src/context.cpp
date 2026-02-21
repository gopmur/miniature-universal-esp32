#include "context.hpp"
#include "services/led.hpp"
#include "services/stm_uart_rx.hpp"

namespace context {

  HttpService http_service;
  DnsService dns_service("192.168.4.1");
  LedService led_service;
  StmUartTxService stm_uart_tx_service(config::stm_uart::port,
                                       config::stm_uart::data_bits,
                                       config::stm_uart::parity,
                                       config::stm_uart::stop_bits,
                                       config::stm_uart::tx_pin,
                                       config::stm_uart::rx_pin,
                                       config::stm_uart::baud_rate,
                                       config::stm_uart::rx_buffer_size);
  StmUartRxService stm_uart_rx_service(config::stm_uart::port);
}