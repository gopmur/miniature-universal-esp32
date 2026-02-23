#pragma once

#include "hal/uart_types.h"

namespace config {

  namespace pins {}

  namespace wifi {
    constexpr auto ssid = "Gopmur ESP-32";
    constexpr auto password = "12345678";
    constexpr auto channel = 6;
    constexpr auto max_connection = 2;
  }

  namespace stm_uart {
    constexpr auto port = UART_NUM_1;
    constexpr auto baud_rate = 115200;
    constexpr auto rx_buffer_size = 256;
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
      constexpr auto priority = 2;
    }
    namespace dns {
      constexpr auto priority = 2;
      constexpr auto stack_size = 4096;
    }

    namespace led {
      constexpr auto priority = 2;
      constexpr auto stack_size = 4096;
      constexpr auto queue_len = 2;
    }
  }
}
