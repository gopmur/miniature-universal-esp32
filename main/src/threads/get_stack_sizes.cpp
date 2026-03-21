#include "threads/get_stack_sizes.hpp"
#include "context/services/http.hpp"
#include "esp_http_server.h"
#include "helper/json.hpp"
#include "portmacro.h"

GetStackSizes::GetStackSizes(const char* name, int priority, int stack_size)
    : ThreadWithArg(name, priority, stack_size) {}

void GetStackSizes::main(httpd_req_t** req_p) {
  auto req = *req_p;
  JsonObject res_json;

  for (int i = 0; i < 8; i++) {
    auto response = http_service.task_stack_size_queue.receive(1000);
    if (!response.has_value()) {
      res_json.set_string("message", "timed out");
      auto res_str = res_json.stringify(); 
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str);
      free(res_str);
      httpd_req_async_handler_complete(req);
      return;
    }
    switch (response->header) {
      case HttpQueueMessageHeader::LED_SERVICE_STACK_SIZE:
        res_json.set_number("led", response->payload.u32);
        break;
      case HttpQueueMessageHeader::IMU_SERVICE_STACK_SIZE:
        res_json.set_number("imu", response->payload.u32);
        break;
      case HttpQueueMessageHeader::ESP_UART_TX_SERVICE_STACK_SIZE:
        res_json.set_number("uartTx", response->payload.u32);
        break;
      case HttpQueueMessageHeader::ESP_UART_RX_SERVICE_STACK_SIZE:
        res_json.set_number("uartRx", response->payload.u32);
        break;
      case HttpQueueMessageHeader::MOTOR_SERVICE_STACK_SIZE:
        res_json.set_number("motor", response->payload.u32);
        break;
      case HttpQueueMessageHeader::CAN_RECV_SERVICE_STACK_SIZE:
        res_json.set_number("canRecv", response->payload.u32);
        break;
      case HttpQueueMessageHeader::SD_SERVICE_STACK_SIZE:
        res_json.set_number("sd", response->payload.u32);
        break;
      case HttpQueueMessageHeader::MONITOR_SERVICE_STACK_SIZE:
        res_json.set_number("monitor", response->payload.u32);
        break;
      default:
        break;
    }
  }

  auto res_str = res_json.stringify();
  httpd_resp_send(req, res_str, HTTPD_RESP_USE_STRLEN);
  httpd_req_async_handler_complete(req);
  free(res_str);
}