#include "context.hpp"
#include "freertos/idf_additions.h"
#include "messages.hpp"

namespace context {

HttpService http_service;
DnsService dns_service("192.168.4.1");
// StmUartService stm_uart_service(config::stm_uart::port,
//                                 config::stm_uart::data_bits,
//                                 config::stm_uart::parity,
//                                 config::stm_uart::stop_bits,
//                                 config::stm_uart::tx_pin,
//                                 config::stm_uart::rx_pin,
//                                 config::stm_uart::baud_rate,
//                                 config::stm_uart::buffer_size);

void init() {
  // context::http_to_stm_uart_queue =
  //     xQueueCreate(4, sizeof(HttpToStmUartMessages));
}

}  // namespace context