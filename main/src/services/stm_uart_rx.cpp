#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include "config.hpp"
#include "context/services/http.hpp"
#include "context/services/ws.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "services/http.hpp"
#include "services/stm_uart/lappl.hpp"

StmUartRxService::StmUartRxService(int priority, uart_port_t port)
    : AbstractService(priority), port(port) {}

void StmUartRxService::main(StmUartRxService* self) {
  while (true) {
    int bytes_read = uart_read_bytes(self->port, self->rx_buffer, 1024, 0);
    if (bytes_read <= 0) {
      vTaskDelay(5);
      continue;
    }
    for (int i = 0; i < bytes_read; i++) {
      auto packet = Lappl::read_stream(self->rx_buffer[i]);
      if (!packet.has_value()) {
        continue;
      }
      HttpQueueMessage http_queue_message;
      if (packet->header.b.resp == 1 &&
          packet->header.b.type == LapplType::READ) {
        switch (static_cast<LapplAddress>(packet->address)) {
          case LapplAddress::RUNNING:
            ESP_LOGI("RECEIVED BYTES", "RUNNING");
            http_queue_message.header = HttpQueueMessageHeader::RUNNING;
            http_queue_message.payload.b = packet->get_uint8();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case LapplAddress::RIGHT_TORQUE:
            ESP_LOGI("RECEIVED BYTES", "RIGHT_TORQUE");
            http_queue_message.header =
                HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case LapplAddress::LEFT_TORQUE:
            ESP_LOGI("RECEIVED BYTES", "LEFT_TORQUE");
            http_queue_message.header =
                HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          case LapplAddress::CONTROL_MODE:
            ESP_LOGI("RECEIVED BYTES", "CONTROL_MODE");
            http_queue_message.header = HttpQueueMessageHeader::MODE;
            http_queue_message.payload.control_mode =
                static_cast<ControlMode>(packet->get_uint8());

            http_service.queue.send(http_queue_message, portMAX_DELAY);
            break;
          default:

            if (ws_service.has_connections()) {
              ws_service.queue.send(packet.value(), portMAX_DELAY);
            }
            // ESP_LOGI("UART RECEIVED",
            //          "%d",
            //          static_cast<uint8_t>(packet->address));
            break;
        }
      }

      else if (packet->header.b.resp == 1 &&
               packet->header.b.type == LapplType::EOC)
      // ESP_LOGI("UART RECEIVED", "EOC");
      {
        if (ws_service.has_connections()) {
          ws_service.queue.send(packet.value(), portMAX_DELAY);
        }
      }
    }
  }
}

void StmUartRxService::start() {
  START_SERVICE("stm_uart_rx_service");
}