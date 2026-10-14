#include "http/modules/update.hpp"
#include <format>
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_https_ota.h"
#include "helper/formats.hpp"
#include "jayson.hpp"
#include "sdkconfig.h"
#include "system_logger.hpp"
#include "tasks/ws.hpp"

extern WebSocketTask ws_task;

bool HttpUpdateModule::ota_busy = false;
HttpUpdateModule::GetCheckHandler HttpUpdateModule::get_check;
HttpUpdateModule::PutHandler HttpUpdateModule::put;

void HttpUpdateModule::PutHandler::main(HttpJsonAsyncHandlerArgs* args) {
  auto req_json = args->json;
  auto req = args->req;

  JsonObject resp_json;
  auto version_result = req_json.get_string("version", &resp_json);
  esp_http_client_config_t config{};

  esp_https_ota_config_t ota_config = {
      .http_config = &config,
      .http_client_init_cb = nullptr,
      .bulk_flash_erase = false,
      .buffer_caps = 0,
      .ota_resumption = true,
  };

  int downloaded;
  int total;

  std::string version;
  std::string uri;

  if (std::holds_alternative<JsonError>(version_result)) {
    send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }

  version = std::get<char*>(version_result);

  uri = std::format("https://{}/firmware/{}.bin", CONFIG_HEXA_OTA_SERVER_ADDRESS, version);

  config.url = uri.c_str();
  config.crt_bundle_attach = esp_crt_bundle_attach;
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = 5000;

  esp_https_ota_handle_t ota_handle;
  esp_err_t err;
  err = esp_https_ota_begin(&ota_config, &ota_handle);
  if (err != ESP_OK) {
    send_message_json(req, "ota begin failed", HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }

  while (esp_https_ota_perform(ota_handle) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
    downloaded = esp_https_ota_get_image_len_read(ota_handle);
    total = esp_https_ota_get_image_size(ota_handle);
    ws_task.send_to_ota_connections(total, downloaded);
  }

  if (err != ESP_OK) {
    ws_task.send_to_ota_connections(false);
    goto cleanup;
  }

  ws_task.send_to_ota_connections(total, total);
  ws_task.send_to_ota_connections(true);

  esp_https_ota_finish(ota_handle);
  send_success_json(req);

cleanup:
  ota_busy = false;
}

void HttpUpdateModule::GetCheckHandler::main(httpd_req_t** req_p) {
  auto req = *req_p;
  esp_http_client_config_t config{};
  auto uri = std::format("https://{}/latest-version", CONFIG_HEXA_OTA_SERVER_ADDRESS);
  config.url = uri.c_str();
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = 5000;
  config.crt_bundle_attach = esp_crt_bundle_attach;

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
  if (status_code != 200) {
    httpd_resp_send_custom_err(req, status_code_str.c_str(), resp_str.c_str());
    goto cleanup;
  }

  httpd_resp_send(req, resp_str.c_str(), resp_str.length());

cleanup:
  esp_http_client_close(client);
  esp_http_client_cleanup(client);
}

esp_err_t HttpUpdateModule::get_ota_status(httpd_req_t* req) {
  JsonObject resp_json;
  resp_json.set("busy", ota_busy);
  return send_json(req, resp_json);
}

void HttpUpdateModule::register_direct_uris() {
  register_async_uri("/check", HTTP_GET, &get_check);
  register_uri("/status", HTTP_GET, get_ota_status);
  register_async_uri("/", HTTP_PUT, &put);
}