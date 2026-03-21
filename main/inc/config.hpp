#pragma once

#include "hal/uart_types.h"

namespace config {

  namespace pins {}

  namespace wifi {
    constexpr auto ssid = "Mini Universal";
    constexpr auto password = "12345678";
    constexpr auto channel = 6;
    constexpr auto max_connection = 3;
  }

  namespace stm_uart {
    constexpr auto port = UART_NUM_1;
    constexpr auto baud_rate = 1200000;
    constexpr auto rx_buffer_size = 4096;
    constexpr auto tx_pin = 17;
    constexpr auto rx_pin = 16;
    constexpr auto data_bits = UART_DATA_8_BITS;
    constexpr auto parity = UART_PARITY_DISABLE;
    constexpr auto stop_bits = UART_STOP_BITS_1;
    constexpr auto packet_length = 5;
  }

  namespace service {
    namespace stm_uart {
      constexpr auto stack_size = 4096;
      constexpr auto priority = 3;
    }
    namespace dns {
      constexpr auto priority = 2;
      constexpr auto stack_size = 4096;
      constexpr auto name = "hexa.lan";
    }

    namespace led {
      constexpr auto priority = 2;
      constexpr auto stack_size = 4096;
      constexpr auto queue_len = 2;
    }

    namespace ws {
      constexpr auto priority = 3;
      constexpr auto stack_size = 4096;
      constexpr auto max_connection = 3;
    }

    namespace monitor {
      constexpr auto priority = 2;
      constexpr auto stack_size = 4096;
    }

  }
}
