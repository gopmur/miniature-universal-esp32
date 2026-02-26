#include "services/ws.hpp"
#include "config.hpp"
#include "context.hpp"
#include "esp_http_server.h"
#include "esp_log.h"
#include "portmacro.h"
#include "service.hpp"

WebSocketService::WebSocketService() {
  this->fd = -1;
};

void WebSocketService::main(WebSocketService* self) {
  char buf[128];
  while (true) {
    self->wait_for_notification();

    while (true) {
      auto value = self->queue.receive(portMAX_DELAY);
      if (self->fd < 0 || !value.has_value()) {
        break;
        ;
      }
      snprintf(buf, sizeof(buf), "counter: %f", value.value());
      httpd_ws_frame_t ws_pkt = {
          .final = true,
          .fragmented = false,
          .type = HTTPD_WS_TYPE_TEXT,
          .payload = (uint8_t*)buf,
          .len = strlen(buf),
      };
      esp_err_t ret =
          httpd_ws_send_frame_async(context::http_service.server_instance,
                                    self->fd,
                                    &ws_pkt);
      if (ret != ESP_OK) {
        ESP_LOGW("WS", "Client disconnected or send failed");
        self->fd = -1;
      }
    }
  }
}

void WebSocketService::start_sending(int fd) {
  this->fd = fd;
  this->notify();
}

void WebSocketService::stop_sending() {
  this->fd = 0;
}

void WebSocketService::start() {
  START_SERVICE("NAME", config::service::ws::priority);
}