#include "services/stm_uart_tx.hpp"
#include <cstdio>
#include "config.hpp"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "portmacro.h"

StmUartTxService::StmUartTxService(uart_port_t port,
                                   uart_word_length_t data_bits,
                                   uart_parity_t parity,
                                   uart_stop_bits_t stop_bits,
                                   int tx_pin,
                                   int rx_pin,
                                   int baud_rate,
                                   int rx_buffer_size)
    : port(port),
      data_bits(data_bits),
      parity(parity),
      stop_bits(stop_bits),
      tx_pin(tx_pin),
      rx_pin(rx_pin),
      baud_rate(baud_rate),
      rx_buffer_size(rx_buffer_size) {}

void StmUartTxService::main(StmUartTxService* self) {
  while (true) {
    auto uart_packet = self->queue.receive(portMAX_DELAY);
    if (!uart_packet)
      continue;
    ESP_LOGI("UART", "SENT %d\n", uart_packet->data()[0]);
    uart_write_bytes(self->port, uart_packet->data(), uart_packet->size());
  }
}

void StmUartTxService::start() {
  ESP_ERROR_CHECK(uart_set_pin(this->port,
                               this->tx_pin,
                               this->rx_pin,
                               UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
  uart_config_t uart_config = {
      .baud_rate = this->baud_rate,
      .data_bits = this->data_bits,
      .parity = this->parity,
      .stop_bits = this->stop_bits,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };
#pragma clang diagnostic pop
  ESP_ERROR_CHECK(uart_param_config(this->port, &uart_config));
  ESP_ERROR_CHECK(
      uart_driver_install(this->port, this->rx_buffer_size, this->rx_buffer_size, 0, nullptr, 0));
  priority = config::service::dns::priority;
  this->thread_id = xTaskCreateStatic(
      reinterpret_cast<void (*)(void*)>(StmUartTxService::main),
      "stm_uart_tx_service",
      stack_size,
      this,
      config::service::stm_uart::priority,
      stack,
      &tcb);
}