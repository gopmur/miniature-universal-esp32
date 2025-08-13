#pragma once

#include "esp_http_server.h"
#include <string.h>

#define MAX_WS_CLIENTS 8
#define INVALID_FD    -1

class HTTPService {
private:
    httpd_handle_t server_instance = nullptr;
    int ws_clients[MAX_WS_CLIENTS] = { INVALID_FD };
    TaskHandle_t ws_task_handle = nullptr;
    bool server_initialized; 



public:
    void add_client(int fd);
    void remove_client(int fd);
    static void ws_send_task(void *pvParameters);
    static esp_err_t websocket_handler(httpd_req_t *req);
    static esp_err_t send_ws_message(httpd_handle_t server, int client_fd, const char *msg);
    void start();
};
