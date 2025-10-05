// #include "services/stm_uart.hpp"
// #include "context.hpp"
// #include "driver/uart.h"
// #include "hal/uart_types.h"
// #include "messages.hpp"
// #include "portmacro.h"

// StmUartService::StmUartService(uart_port_t port,
//                                uart_word_length_t data_bits,
//                                uart_parity_t parity,
//                                uart_stop_bits_t stop_bits,
//                                int tx_pin,
//                                int rx_pin,
//                                int baud_rate,
//                                int buffer_size)
//     : port(port),
//       data_bits(data_bits),
//       parity(parity),
//       stop_bits(stop_bits),
//       tx_pin(tx_pin),
//       rx_pin(rx_pin),
//       baud_rate(baud_rate),
//       buffer_size(buffer_size) {}

// void StmUartService::main(StmUartService* service) {
//   while (true) {
//     HttpToStmUartSignal signal;
//     xQueueReceive(context::http_to_stm_uart_queue, &signal, portMAX_DELAY);
//   }
// }

// void StmUartService::start() {
//   uart_set_pin(this->port, this->tx_pin, this->rx_pin, UART_PIN_NO_CHANGE,
//                UART_PIN_NO_CHANGE);

// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wmissing-field-initializers"
//   uart_config_t uart_config = {
//       .baud_rate = this->baud_rate,
//       .data_bits = this->data_bits,
//       .parity = this->parity,
//       .stop_bits = this->stop_bits,
//       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//       .rx_flow_ctrl_thresh = 0,
//   };
// #pragma clang diagnostic pop
//   uart_param_config(this->port, &uart_config);
//   uart_driver_install(this->port, this->buffer_size * 2, 0, 0, nullptr, 0);
//   priority = config::service::dns::priority;
//   this->thread_id = xTaskCreateStatic(
//       reinterpret_cast<void (*)(void*)>(StmUartService::main), "dns_service",
//       stack_size, this, config::service::stm_uart::stack_size, stack, &tcb);
// }