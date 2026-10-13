#include "http/modules/system.hpp"
#include <format>
#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "jaythread/sync.hpp"
#include "nvs_flash.h"
#include "sd_protocol_types.h"
#include "system_logger.hpp"

extern sdmmc_card_t* card;

HttpSystemModule::GetCheckUpdateHandler HttpSystemModule::get_check_update;

void HttpSystemModule::GetCheckUpdateHandler::main(httpd_req_t** req_p) {
  auto req = *req_p;
  esp_http_client_config_t config{};
  config.url = "http://192.168.4.2:3000/latest-version";
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = 5000;

  esp_http_client_handle_t client = esp_http_client_init(&config);

  uint32_t status_code = 200;
  int resp_len;
  std::string resp_str;
  char* resp_buffer;
  std::string status_code_str;

  auto result = esp_http_client_open(client, 0);
  if (result != ESP_OK) {
    send_message_json(req, "could not reach server", HTTPD_500_INTERNAL_SERVER_ERROR);
    goto cleanup;
  }

  result = esp_http_client_fetch_headers(client);
  resp_len = esp_http_client_get_content_length(client);
  resp_buffer = new char[resp_len + 1];
  esp_http_client_read(client, resp_buffer, resp_len);
  resp_buffer[resp_len] = 0;
  resp_str = std::string(resp_buffer);
  delete[] resp_buffer;
  status_code = esp_http_client_get_status_code(client);
  status_code_str = std::format("{}", status_code);
  httpd_resp_send_custom_err(req, status_code_str.c_str(), resp_str.c_str());

cleanup:
  esp_http_client_close(client);
  esp_http_client_cleanup(client);
}

esp_err_t HttpSystemModule::get_version(httpd_req_t* req) {
  const esp_app_desc_t* app_desc = esp_app_get_description();
  JsonObject resp_json;
  resp_json.set("version", app_desc->version);
  resp_json.set("idfVersion", app_desc->idf_ver);
  resp_json.set("buildDate", app_desc->date);
  return send_json(req, resp_json);
}

esp_err_t HttpSystemModule::get_factory_reset(httpd_req_t* req) {
  JsonObject resp_json;
  auto status = nvs_flash_erase();
  if (status != ESP_OK) {
    return send_message_json(req, "failed to erase flash", HTTPD_500_INTERNAL_SERVER_ERROR);
  }
  SystemLogger::close_log_file();
  status = esp_vfs_fat_sdcard_format("/sd", card);
  if (status != ESP_OK) {
    resp_json.set("message", "failed to format sd card");
    return send_message_json(req, "failed to format sd card", HTTPD_500_INTERNAL_SERVER_ERROR);
  }
  resp_json.set("message", "restarting in 1s");
  send_json(req, resp_json);
  Sync::sleep(1000);
  esp_restart();
  return ESP_OK;
};

esp_err_t HttpSystemModule::get_restart(httpd_req_t* req) {
  send_message_json(req, "restarting in 1 seconds");
  Sync::sleep(1000);
  esp_restart();
  return ESP_OK;
}

void HttpSystemModule::register_direct_uris() {
  register_uri("/restart", HTTP_GET, get_restart);
  register_uri("/factory-reset", HTTP_GET, get_factory_reset);
  register_uri("/version", HTTP_GET, get_version);
  register_async_uri("/check-update", HTTP_GET, &get_check_update);
}