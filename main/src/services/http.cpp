#include "services/http.hpp"

#include "http_assets.hpp"

void HTTPService::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
}
