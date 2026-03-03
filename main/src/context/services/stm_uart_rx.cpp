#include "services/stm_uart_rx.hpp"
#include "config.hpp"

StmUartRxService stm_uart_rx_service(config::service::stm_uart::priority,
                                     config::stm_uart::port);