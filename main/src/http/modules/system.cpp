#include "http/modules/system.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "jaythread/sync.hpp"
#include "nvs_flash.h"
#include "sd_protocol_types.h"
#include "system_logger.hpp"

extern sdmmc_card_t* card;

esp_err_t HttpSystemModule::get_factory_reset(httpd_req_t* req) {
  set_header(req);
  JsonObject resp_json;
  auto status = nvs_flash_erase();
  if (status != ESP_OK) {
    resp_json.set("message", "failed to erase flash");
    return send_json(req, resp_json, HTTPD_500_INTERNAL_SERVER_ERROR);
  }
  SystemLogger::close_log_file();
  status = esp_vfs_fat_sdcard_format("/sd", card);
  if (status != ESP_OK) {
    resp_json.set("message", "failed to format sd card");
    return send_json(req, resp_json, HTTPD_500_INTERNAL_SERVER_ERROR);
  }
  resp_json.set("message", "restarting in 1s");
  send_json(req, resp_json);
  Sync::sleep(1000);
  esp_restart();
  return ESP_OK;
};

esp_err_t HttpSystemModule::get_restart(httpd_req_t* req) {
  set_header(req);
  ESP_ERROR_CHECK(httpd_resp_send(req, nullptr, 0));
  Sync::sleep(1000);
  esp_restart();
  return ESP_OK;
}

void HttpSystemModule::register_direct_uris() {
  register_uri("/restart", HTTP_GET, get_restart);
  register_uri("/factory-reset", HTTP_GET, get_factory_reset);
}