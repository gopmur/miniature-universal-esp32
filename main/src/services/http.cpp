#include "services/http.hpp"
#include <esp_log.h>
#include "http_assets.hpp"
#define MAX_WS_CLIENTS 8
static int ws_client_fds[MAX_WS_CLIENTS] = { -1 }; // -1 = empty slot


void HTTPService::add_client(int fd) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (ws_client_fds[i] == -1) {
            ws_client_fds[i] = fd;
            ESP_LOGI("WS", "Added client fd=%d at slot=%d", fd, i);
            return;
        }
    }
    ESP_LOGW("WS", "Max WebSocket clients reached");
}

void HTTPService::remove_client(int fd) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (ws_client_fds[i] == fd) {
            ws_client_fds[i] = -1;
            ESP_LOGI("WS", "Removed client fd=%d", fd);
            return;
        }
    }
}

esp_err_t HTTPService::websocket_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI("WS", "Handshake done");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Get frame length
    ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, 0));
    uint8_t *buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    ws_pkt.payload = buf;

    // Receive actual data
    ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len));
    ESP_LOGI("WEBSOCKET", "Got message: %s", (char *)ws_pkt.payload);

    free(buf);
    return ESP_OK;
}

esp_err_t HTTPService::send_ws_message(httpd_handle_t server, int client_fd, const char *msg) {
    if (!server || !msg) return ESP_FAIL;
    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)msg;
    ws_pkt.len = strlen(msg);
    return httpd_ws_send_frame_async(server, client_fd, &ws_pkt);
}

// For test purposes only
void HTTPService::ws_send_task(void *pvParameters) {
    HTTPService *self = static_cast<HTTPService*>(pvParameters);
    const char *numbers[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    size_t i = 0;

    while (true) {
        httpd_handle_t srv = self->server_instance;
        if (!srv) {
            ESP_LOGW("WS", "Server instance not initialized");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        size_t max = 8;
        int fds[8] = {0};
        if (max > 8) {
            ESP_LOGW("WS", "Client list exceeds array size");
            max = 8;
        }

        ESP_LOGI("WS", "Before get_client_list: srv=%p, max=%zu", srv, max);
        if (httpd_get_client_list(srv, &max, fds) == ESP_OK) {
            for (size_t k = 0; k < max; ++k) {
                int fd = fds[k];
                if (fd <= 0) continue;

                // Check if the client is a WebSocket client
                httpd_ws_client_info_t st = httpd_ws_get_fd_info(srv, fd);
                if (st == HTTPD_WS_CLIENT_WEBSOCKET) {
                    esp_err_t err = send_ws_message(srv, fd, numbers[i]);
                    if (err != ESP_OK) {
                        ESP_LOGW("WS", "send failed to fd=%d err=%d", fd, (int)err);
                    }
                }
                taskYIELD();
            }
        } else {
            ESP_LOGW("WS", "Failed to get client list");
        }

        i = (i + 1) % 10;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void HTTPService::start() {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.max_uri_handlers = 10;
    cfg.server_port = 80;
    ESP_ERROR_CHECK(httpd_start(&server_instance, &cfg));

    http_server_register_assets(server_instance);  // <-- restore this

    httpd_uri_t ws_uri = {};
    ws_uri.uri = "/ws";
    ws_uri.method = HTTP_GET;
    ws_uri.handler = websocket_handler;
    ws_uri.user_ctx = this;
    ws_uri.is_websocket = true;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server_instance, &ws_uri));

    // if (xTaskCreate(ws_send_task, "ws_send_task", 1024, this, 5, &ws_task_handle) != pdPASS) {
    //     ESP_LOGE("HTTPService", "Failed to create ws_send_task");
    // }
}