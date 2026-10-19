#pragma once

#include "esp_http_server.h"
#include "http/module.hpp"
#include "system_logger.hpp"

extern const uint8_t server_cert_pem_start[] asm("_binary_ecc_cert_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ecc_cert_crt_end");
extern const uint8_t server_key_pem_start[] asm("_binary_ecc_private_key_start");
extern const uint8_t server_key_pem_end[] asm("_binary_ecc_private_key_end");

class HttpServer {
  MAKE_LOGGABLE("http_server");

  private:
  HttpModule* root_module;
  static void on_close(httpd_handle_t server, int sockfd);

  public:
  httpd_handle_t server_instance;
  HttpServer(HttpModule* root_module);
  void start();
};
