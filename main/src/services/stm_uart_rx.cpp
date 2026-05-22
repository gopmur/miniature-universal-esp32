#include "services/stm_uart_rx.hpp"
#include <cstdio>
#include <optional>
#include "config.hpp"
#include "context/services/http.hpp"
#include "context/services/ws.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "services/http.hpp"
#include "services/stm_uart/ssp.hpp"

StmUartRxService::StmUartRxService(int priority, uart_port_t port)
    : Service(priority, "stm_uart_rx"), port(port) {}

std::optional<HttpQueueMessageHeader> StmUartRxService::ssp_address_to_http_queue_message_header(
    SspAddress ssp_address) {
  switch (ssp_address) {
    case SspAddress::RUNNING:
      return HttpQueueMessageHeader::RUNNING;
    case SspAddress::RIGHT_TORQUE:
      return HttpQueueMessageHeader::RIGHT_MANUAL_TORQUE;
    case SspAddress::LEFT_TORQUE:
      return HttpQueueMessageHeader::LEFT_MANUAL_TORQUE;
    case SspAddress::CONTROL_MODE:
      return HttpQueueMessageHeader::MODE;
    case SspAddress::LED_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::LED_SERVICE_STACK_SIZE;
    case SspAddress::IMU_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::IMU_SERVICE_STACK_SIZE;
    case SspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::ESP_UART_TX_SERVICE_STACK_SIZE;
    case SspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::ESP_UART_RX_SERVICE_STACK_SIZE;
    case SspAddress::MOTOR_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::MOTOR_SERVICE_STACK_SIZE;
    case SspAddress::CAN_RECV_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::CAN_RECV_SERVICE_STACK_SIZE;
    case SspAddress::SD_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::SD_SERVICE_STACK_SIZE;
    case SspAddress::MONITOR_SERVICE_STACK_SIZE:
      return HttpQueueMessageHeader::MONITOR_SERVICE_STACK_SIZE;
    case SspAddress::AUTOMATIC_RIGHT_VELOCITY_THRESHOLD:
      return HttpQueueMessageHeader::AUTOMATIC_RIGHT_VELOCITY_THRESHOLD;
    case SspAddress::AUTOMATIC_LEFT_VELOCITY_THRESHOLD:
      return HttpQueueMessageHeader::AUTOMATIC_LEFT_VELOCITY_THRESHOLD;
    case SspAddress::AUTOMATIC_RIGHT_TORQUE:
      return HttpQueueMessageHeader::AUTOMATIC_RIGHT_TORQUE;
    case SspAddress::AUTOMATIC_LEFT_TORQUE:
      return HttpQueueMessageHeader::AUTOMATIC_LEFT_TORQUE;
    case SspAddress::AUTOMATIC_RIGHT_TIMEOUT:
      return HttpQueueMessageHeader::AUTOMATIC_RIGHT_TIMEOUT;
    case SspAddress::AUTOMATIC_LEFT_TIMEOUT:
      return HttpQueueMessageHeader::AUTOMATIC_LEFT_TIMEOUT;
    case SspAddress::SEMIAUTOMATIC_WEAK_LEG:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_WEAK_LEG;
    case SspAddress::SEMIAUTOMATIC_START_ASSIST_ANGLE:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_START_ASSIST_ANGLE;
    case SspAddress::SEMIAUTOMATIC_STOP_ASSIST_ANGLE:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_STOP_ASSIST_ANGLE;
    case SspAddress::SEMIAUTOMATIC_RIGHT_TORQUE:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TORQUE;
    case SspAddress::SEMIAUTOMATIC_LEFT_TORQUE:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TORQUE;
    case SspAddress::SEMIAUTOMATIC_RIGHT_DELAY:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_DELAY;
    case SspAddress::SEMIAUTOMATIC_LEFT_DELAY:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_DELAY;
    case SspAddress::SEMIAUTOMATIC_LEFT_TIMEOUT:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_LEFT_TIMEOUT;
    case SspAddress::SEMIAUTOMATIC_RIGHT_TIMEOUT:
      return HttpQueueMessageHeader::SEMIAUTOMATIC_RIGHT_TIMEOUT;
    case SspAddress::SMART_RIGHT_TORQUE:
      return HttpQueueMessageHeader::SMART_RIGHT_TORQUE;
    case SspAddress::SMART_LEFT_TORQUE:
      return HttpQueueMessageHeader::SMART_LEFT_TORQUE;
    default:
      return std::nullopt;
  }
}

void log_packet(SspPacket packet) {
  switch (packet.address) {
    case SspAddress::ZERO:
      ESP_LOGW("UART", "ZERO received");
      break;

    case SspAddress::RAND:
      ESP_LOGI("UART", "RAND received");
      break;

    case SspAddress::RUNNING:
      ESP_LOGI("UART", "RUNNING received");
      break;

    case SspAddress::LEFT_TORQUE:
      ESP_LOGI("UART", "LEFT_TORQUE received");
      break;

    case SspAddress::RIGHT_TORQUE:
      ESP_LOGI("UART", "RIGHT_TORQUE received");
      break;

    case SspAddress::CONTROL_MODE:
      ESP_LOGI("UART", "CONTROL_MODE received");
      break;

    case SspAddress::IMU_GX:
      ESP_LOGI("UART", "IMU_GX received");
      break;

    case SspAddress::IMU_GY:
      ESP_LOGI("UART", "IMU_GY received");
      break;

    case SspAddress::IMU_GZ:
      ESP_LOGI("UART", "IMU_GZ received");
      break;

    case SspAddress::LED_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "LED_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::IMU_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "IMU_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::ESP_UART_TX_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::ESP_UART_RX_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::MOTOR_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "MOTOR_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::CAN_RECV_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::SD_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "SD_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::MONITOR_SERVICE_CPU_USAGE:
      ESP_LOGI("UART", "MONITOR_SERVICE_CPU_USAGE received");
      break;

    case SspAddress::LED_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "LED_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::IMU_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "IMU_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::ESP_UART_TX_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::ESP_UART_RX_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::MOTOR_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "MOTOR_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::CAN_RECV_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::SD_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "SD_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::MONITOR_SERVICE_MIN_STACK_FREE:
      ESP_LOGI("UART", "MONITOR_SERVICE_MIN_STACK_FREE received");
      break;

    case SspAddress::LED_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "LED_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::IMU_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "IMU_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "ESP_UART_TX_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "ESP_UART_RX_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::MOTOR_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "MOTOR_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::CAN_RECV_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "CAN_RECV_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::SD_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "SD_SERVICE_STACK_SIZE received");
      break;

    case SspAddress::MONITOR_SERVICE_STACK_SIZE:
      ESP_LOGI("UART", "MONITOR_SERVICE_STACK_SIZE received");
      break;

      // case SspAddress::HEAP_USAGE:
      //   ESP_LOGI("UART", "HEAP_USAGE received");
      //   break;

      // case SspAddress::MAX_HEAP_USAGE:
      //   ESP_LOGI("UART", "MAX_HEAP_USAGE received");
      //   break;

      // case SspAddress::HEAP_SIZE:
      //   ESP_LOGI("UART", "HEAP_SIZE received");
      //   break;

    case SspAddress::RTC_TIME:
      ESP_LOGI("UART", "RTC_TIME received");
      break;

    case SspAddress::RTC_DATE:
      ESP_LOGI("UART", "RTC_DATE received");
      break;

    case SspAddress::MOTOR_POS_LEFT:
      ESP_LOGI("UART", "MOTOR_POS_LEFT received");
      break;

    case SspAddress::MOTOR_POS_RIGHT:
      ESP_LOGI("UART", "MOTOR_POS_RIGHT received");
      break;

    case SspAddress::ADDRESS_COUNT:
      ESP_LOGI("UART", "ADDRESS_COUNT received");
      break;

    default:
      ESP_LOGW("UART", "Unknown address");
      break;
  }
}

void StmUartRxService::main() {
  while (true) {
    int bytes_read =
        uart_read_bytes(this->port, this->rx_buffer, config::stm_uart::rx_buffer_size, 4);
    if (bytes_read <= 0) {
      continue;
    }
    for (int i = 0; i < bytes_read; i++) {
      auto packet = Ssp::read_stream(this->rx_buffer[i]);
      if (!packet.has_value()) {
        continue;
      }
      HttpQueueMessage http_queue_message;
      auto http_queue_message_header_result =
          ssp_address_to_http_queue_message_header(packet->address);
      if (packet->header.b.resp == 1 && packet->header.b.type == SspType::READ) {
        switch (static_cast<SspAddress>(packet->address)) {
          case SspAddress::RUNNING:
            if (!http_queue_message_header_result.has_value()) {
              break;
            }
            http_queue_message.header = http_queue_message_header_result.value();
            http_queue_message.payload.b = packet->get_uint8();
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case SspAddress::CONTROL_MODE:
            if (!http_queue_message_header_result.has_value()) {
              break;
            }
            http_queue_message.header = http_queue_message_header_result.value();
            http_queue_message.payload.control_mode = static_cast<ControlMode>(packet->get_uint8());
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case SspAddress::SEMIAUTOMATIC_WEAK_LEG:
            if (!http_queue_message_header_result.has_value()) {
              break;
            }
            http_queue_message.header = http_queue_message_header_result.value();
            http_queue_message.payload.leg = static_cast<Leg>(packet->get_uint8());
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case SspAddress::RIGHT_TORQUE:
          case SspAddress::LEFT_TORQUE:
          case SspAddress::AUTOMATIC_LEFT_TIMEOUT:
          case SspAddress::AUTOMATIC_RIGHT_TIMEOUT:
          case SspAddress::AUTOMATIC_LEFT_TORQUE:
          case SspAddress::AUTOMATIC_RIGHT_TORQUE:
          case SspAddress::AUTOMATIC_LEFT_VELOCITY_THRESHOLD:
          case SspAddress::AUTOMATIC_RIGHT_VELOCITY_THRESHOLD:
          case SspAddress::SEMIAUTOMATIC_START_ASSIST_ANGLE:
          case SspAddress::SEMIAUTOMATIC_STOP_ASSIST_ANGLE:
          case SspAddress::SEMIAUTOMATIC_LEFT_DELAY:
          case SspAddress::SEMIAUTOMATIC_LEFT_TORQUE:
          case SspAddress::SEMIAUTOMATIC_LEFT_TIMEOUT:
          case SspAddress::SEMIAUTOMATIC_RIGHT_DELAY:
          case SspAddress::SEMIAUTOMATIC_RIGHT_TORQUE:
          case SspAddress::SEMIAUTOMATIC_RIGHT_TIMEOUT:
          case SspAddress::SMART_LEFT_TORQUE:
          case SspAddress::SMART_RIGHT_TORQUE:
            if (!http_queue_message_header_result.has_value()) {
              break;
            }
            http_queue_message.header = http_queue_message_header_result.value();
            http_queue_message.payload.f = packet->get_float();
            http_service.state_queue.send(http_queue_message, portMAX_DELAY);
            break;
          case SspAddress::LED_SERVICE_STACK_SIZE:
          case SspAddress::IMU_SERVICE_STACK_SIZE:
          case SspAddress::ESP_UART_TX_SERVICE_STACK_SIZE:
          case SspAddress::ESP_UART_RX_SERVICE_STACK_SIZE:
          case SspAddress::MOTOR_SERVICE_STACK_SIZE:
          case SspAddress::CAN_RECV_SERVICE_STACK_SIZE:
          case SspAddress::SD_SERVICE_STACK_SIZE:
          case SspAddress::MONITOR_SERVICE_STACK_SIZE: {
            if (!http_queue_message_header_result.has_value()) {
              break;
            }
            http_queue_message.header = http_queue_message_header_result.value();
            http_queue_message.payload.u32 = packet->get_uint32();
            http_service.task_stack_size_queue.send(http_queue_message, portMAX_DELAY);
            break;
          }

          default:
            if (ws_service.has_connections()) {
              if (ws_service.uart_streams_enabled()) {
                ws_service.queue.send(packet.value(), portMAX_DELAY);
              } else {
                ws_service.queue.flush();
              }
            }
            break;
        }
      }

      else if (packet->header.b.resp == 1 && packet->header.b.type == SspType::EOC) {
        if (ws_service.has_connections()) {
          if (ws_service.uart_streams_enabled()) {
            ws_service.queue.send(packet.value(), portMAX_DELAY);
          } else {
            ws_service.queue.flush();
          }
        }
      }
    }
  }
}