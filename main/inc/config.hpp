#pragma once

#include "hal/uart_types.h"

namespace config {
  namespace wifi {
    constexpr auto ssid = "Gopmur ESP-32";
    constexpr auto password = "12345678";
    constexpr auto channel = 6;
    constexpr auto max_connection = 2;
  }

  namespace stm_uart {
    constexpr auto port = UART_NUM_1;
    constexpr auto baud_rate = 115200;
    constexpr auto buffer_size = 1024;
    constexpr auto tx_pin = 4;
    constexpr auto rx_pin = 5;
    constexpr auto data_bits = UART_DATA_8_BITS;
    constexpr auto parity = UART_PARITY_DISABLE;
    constexpr auto stop_bits = UART_STOP_BITS_1;
  }

  namespace service {
    namespace stm_uart {
      constexpr auto stack_size = 1024;
      constexpr auto priority = 2;
    }
    namespace dns {
      constexpr auto priority = 2;
      constexpr auto stack_size = 1024;
    }

    namespace led {}
  }
}
