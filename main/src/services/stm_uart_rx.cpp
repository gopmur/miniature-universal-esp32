#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include <optional>
#include "config.hpp"
#include "context/services/http.hpp"
#include "context/services/ws.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "services/http.hpp"
#include "services/stm_uart/rssp.hpp"

StmUartRxService::StmUartRxService(int priority, uart_port_t port)
    : AbstractService(priority, "stm_uart_rx"), port(port) {}

std::optional<HttpQueueMessageHeader>
StmUartRxService::rssp_address_to_http_queue_message_header(
    RsspAddress rssp_address) {
  switch (rssp_address) {
    case RsspAddress::RUNNING:
      return HttpQueueMessageHeader::RUNNING;
    case RsspAddress::RIGHT_TORQUE:
      return HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
    case RsspAddress::LEFT_TORQUE:
      return HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
    case RsspAddress::CONTROL_MODE:
      return HttpQueueMessageHeader::MODE;
    case RsspAddress::LED_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::LED_SERVICE_STACK_SIZE;
    case RsspAddress::IMU_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::IMU_SERVICE_STACK_SIZE;
    case RsspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::ESP_UART_TX_SERVICE_STACK_SIZE;
    case RsspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::ESP_UART_RX_SERVICE_STACK_SIZE;
    case RsspAddress::MOTOR_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::MOTOR_SERVICE_STACK_SIZE;
    case RsspAddress::CAN_RECV_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::CAN_RECV_SERVICE_STACK_SIZE;
    case RsspAddress::SD_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::SD_SERVICE_STACK_SIZE;
    case RsspAddress::MONITOR_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::MONITOR_SERVICE_STACK_SIZE;
    default:
      return std::nullopt;
  }
}

void log_packet(RsspPacket packet) {
  switch (packet.address) {
    case RsspAddress::ZERO:
      ESP_LOGI("UART", "ZERO received");
      break;

    case RsspAddress::RAND:
      ESP_LOGI("UART", "RAND received");
      break;

    case RsspAddress::RUNNING:
      ESP_LOGI("UART", "RUNNING received");
      break;

    case RsspAddress::LEFT_TORQUE:
      ESP_LOGI("UART", "LEFT_TORQUE received");
      break;

    case RsspAddress::RIGHT_TORQUE:
      ESP_LOGI("UART", "RIGHT_TORQUE received");
      break;

    case RsspAddress::CONTROL_MODE:
      ESP_LOGI("UART", "CONTROL_MODE received");
      break;

    case RsspAddress::IMU_GX:
      ESP_LOGI("UART", "IMU_GX received");
      break;

    case RsspAddress::IMU_GY:
      ESP_LOGI("UART", "IMU_GY received");
      break;

    case RsspAddress::IMU_GZ:
      ESP_LOGI("UART", "IMU_GZ received");
      break;

    case RsspAddress::LED_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "LED_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::IMU_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "IMU_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::ESP_UART_TX_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::ESP_UART_RX_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::MOTOR_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "MOTOR_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::CAN_RECV_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::SD_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "SD_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::MONITOR_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "MONITOR_SERVICE_CPU_USAGE received");
      break;

    case RsspAddress::LED_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "LED_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::IMU_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "IMU_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::ESP_UART_TX_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::ESP_UART_RX_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::MOTOR_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "MOTOR_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::CAN_RECV_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::SD_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "SD_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::MONITOR_SERVICE_MAX_STACK_USAGE:
      ESP_LOGI("UART", "MONITOR_SERVICE_MAX_STACK_USAGE received");
      break;

    case RsspAddress::LED_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "LED_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::IMU_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "IMU_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::MOTOR_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "MOTOR_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::CAN_RECV_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::SD_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "SD_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::MONITOR_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "MONITOR_SERVICE_STACK_SIZE received");
      break;

    case RsspAddress::HEAP_USAGE:
      ESP_LOGI("UART", "HEAP_USAGE received");
      break;

    case RsspAddress::MAX_HEAP_USAGE:
      ESP_LOGI("UART", "MAX_HEAP_USAGE received");
      break;

    case RsspAddress::HEAP_SIZE:
      ESP_LOGI("UART", "HEAP_SIZE received");
      break;

    case RsspAddress::RTC_TIME:
      ESP_LOGI("UART", "RTC_TIME received");
      break;

    case RsspAddress::RTC_DATE:
      ESP_LOGI("UART", "RTC_DATE received");
      break;

    case RsspAddress::MOTOR_POS_LEFT:
      ESP_LOGI("UART", "MOTOR_POS_LEFT received");
      break;

    case RsspAddress::MOTOR_POS_RIGHT:
      ESP_LOGI("UART", "MOTOR_POS_RIGHT received");
      break;

    case RsspAddress::ADDRESS_COUNT:
      ESP_LOGI("UART", "ADDRESS_COUNT received");
      break;

    default:
      ESP_LOGW("UART", "Unknown address");
      break;
  }
}

void StmUartRxService::main() {
  while (true) {
    int bytes_read = uart_read_bytes(this->port,
                                     this->rx_buffer,
                                     config::stm_uart::rx_buffer_size,
                                     0);
    if (bytes_read <= 0) {
      vTaskDelay(20);
      continue;
    }
    // ESP_LOGW("UART", "Bytes received");
    for (int i = 0; i < bytes_read; i++) {
      auto packet = Rssp::read_stream(this->rx_buffer[i]);
      if (!packet.has_value()) {
        continue;
      }
      // log_packet(packet.value());
      HttpQueueMessage http_queue_message;
      if (packet->header.b.resp == 1 &&
          packet->header.b.type == RsspType::READ) {
        switch (static_cast<RsspAddress>(packet->address)) {
          case RsspAddress::RUNNING:
            http_queue_message.header = HttpQueueMessageHeader::RUNNING;
            http_queue_message.payload.b = packet->get_uint8();
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::RIGHT_TORQUE:
            http_queue_message.header =
                HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::LEFT_TORQUE:
            http_queue_message.header =
                HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
            http_queue_message.payload.f = packet->get_float();
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::CONTROL_MODE:
            http_queue_message.header = HttpQueueMessageHeader::MODE;
            http_queue_message.payload.control_mode =
                static_cast<ControlMode>(packet->get_uint8());
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case RsspAddress::LED_SERVICE_STACK_SIZE:
          case RsspAddress::IMU_SERVICE_STACK_SIZE:
          case RsspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
          case RsspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
          case RsspAddress::MOTOR_SERVICE_STACK_SIZE:
          case RsspAddress::CAN_RECV_SERVICE_STACK_SIZE:
          case RsspAddress::SD_SERVICE_STACK_SIZE:
          case RsspAddress::MONITOR_SERVICE_STACK_SIZE: {
            auto http_queue_message_header =
                rssp_address_to_http_queue_message_header(packet->address);
            if (!http_queue_message_header) {
              break;
            }
            http_queue_message.header = http_queue_message_header.value();
            http_queue_message.payload.u32 = packet->get_uint32();
            http_service.task_stack_size_queue.send(http_queue_message,
                                                    portMAX_DELAY);
            break;
          }

          default:
            if (ws_service.has_connections()) {
              if (ws_service.uart_streams_enabled()) {
                ws_service.queue.send(packet.value(), 1000);
              } else {
                ws_service.queue.flush();
              }
            }
            break;
        }
      }

      else if (packet->header.b.resp == 1 &&
               packet->header.b.type == RsspType::EOC) {
        if (ws_service.has_connections()) {
          if (ws_service.uart_streams_enabled()) {
            ws_service.queue.send(packet.value(), 1000);
          } else {
            ws_service.queue.flush();
          }
        }
      }
    }
  }
}