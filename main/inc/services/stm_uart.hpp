#pragma once

#include "config.hpp"
#include "hal/uart_types.h"
#include "ipc/queue.hpp"
#include "service.hpp"
#include "stm_uart/packet.hpp"

class StmUartService : public Service<config::service::stm_uart::stack_size> {
  private:
  const uart_port_t port;
  const uart_word_length_t data_bits;
  const uart_parity_t parity;
  const uart_stop_bits_t stop_bits;
  const int tx_pin;
  const int rx_pin;
  const int baud_rate;
  const int rx_buffer_size;

  static void main(StmUartService* service);

  public:
  Queue<UartPacket, 32> queue;
  StmUartService(uart_port_t port,
                 uart_word_length_t data_bits,
                 uart_parity_t parity,
                 uart_stop_bits_t stop_bits,
                 int tx_pin,
                 int rx_pin,
                 int baud_rate,
                 int buffer_size);

  void start();
};