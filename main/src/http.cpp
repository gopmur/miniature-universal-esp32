#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "http.hpp"

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_https_server.h"
#include "http_assets.hpp"

#include "http/module.hpp"
#include "sdkconfig.h"
#include "system_logger.hpp"
#include "tasks/ws.hpp"


// // esp_err_t get_ota_status(httpd_req_t* req) {
// //   set_header(req);
// //   JsonObject resp_json;
// //   resp_json.set("busy", ota_busy);
// //   auto resp_str = resp_json.stringify();
// //   httpd_resp_send(req, resp_str.c_str(), HTTPD_RESP_USE_STRLEN);
// //   return ESP_OK;
// // }


extern WebSocketTask ws_task;

HttpServer::HttpServer(HttpModule* root_module) : root_module(root_module) {};

void HttpServer::on_close(httpd_handle_t server, int sockfd) {
  LOGI("tcp connection closed");
  ws_task.remove_connection(sockfd);
}

void HttpServer::start() {
#ifdef CONFIG_HEXA_HTTP_USE_HTTPS
  httpd_ssl_config_t https_config = HTTPD_SSL_CONFIG_DEFAULT();
  https_config.httpd.keep_alive_enable = true;
  https_config.httpd.max_open_sockets = CONFIG_HEXA_HTTP_MAX_OPEN_SOCKET;
  https_config.httpd.max_uri_handlers = CONFIG_HEXA_HTTP_MAX_URI_HANDLER_COUNT;
  https_config.servercert = server_cert_pem_start;
  https_config.servercert_len = strlen((char*)server_cert_pem_start) + 1;
  https_config.prvtkey_pem = server_key_pem_start;
  https_config.prvtkey_len = strlen((char*)server_key_pem_start) + 1;
  ESP_ERROR_CHECK(httpd_ssl_start(&server_instance, &https_config));
#else
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.keep_alive_enable = true;
  http_config.max_open_sockets = CONFIG_HEXA_HTTP_MAX_OPEN_SOCKET;
  http_config.max_uri_handlers = CONFIG_HEXA_HTTP_MAX_URI_HANDLER_COUNT;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
#endif
  http_server_register_assets(server_instance);
  root_module->register_uris(server_instance);
}
