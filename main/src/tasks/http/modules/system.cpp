#include "tasks/http/modules/system.hpp"
#include "esp_err.h"
#include "jaythread/sync.hpp"

esp_err_t HttpSystemModule::get_restart(httpd_req_t* req) {
  set_header(req);
  ESP_ERROR_CHECK(httpd_resp_send(req, nullptr, 0));
  Sync::sleep(1000);
  esp_restart();
  return ESP_OK;
}

void HttpSystemModule::register_direct_uris() {
  register_uri("/restart", HTTP_GET, get_restart);
}