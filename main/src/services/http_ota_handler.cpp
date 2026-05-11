#include "services/http_ota_handler.hpp"
#include <variant>
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "helper/json.hpp"

extern const char cert_pem_start[] asm("_binary_cert_pem_start");

HttpOtaHandlerService::HttpOtaHandlerService(int priority)
    : Service(priority, "http_ota_handler") {}

void HttpOtaHandlerService::main() {
  while (true) {
    auto req_result = queue.receive();
    if (!req_result.has_value()) {
      continue;
    }
    auto req = req_result.value();

    char* req_body = static_cast<char*>(malloc(req->content_len + 1));
    httpd_req_recv(req, req_body, req->content_len);
    auto req_json_result = JsonObject::parse(req_body);
    free(req_body);

    JsonObject res_json;
    if (std::holds_alternative<JsonError>(req_json_result)) {
      res_json.set("message", "parse error");
      auto res_str = res_json.stringify();
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
      httpd_req_async_handler_complete(req);
      continue;
    }

    auto req_json = std::get<JsonObject>(req_json_result);
    auto core_version_result = req_json.get_string("core", &res_json);
    auto api_version_result = req_json.get_string("api", &res_json);

    if (std::holds_alternative<char*>(core_version_result) &&
        std::holds_alternative<char*>(api_version_result)) {
      auto core_version = std::get<char*>(core_version_result);
      auto api_version = std::get<char*>(api_version_result);

      char update_file_url[128];
      snprintf(update_file_url,
               64,
               "https://10.85.100.185:3001/firmware/core-%s-api-%s.bin",
               core_version,
               api_version);

      ESP_LOGI("OTA", "%s", update_file_url);

      esp_http_client_config_t config{};
      config.url = update_file_url;
      config.cert_pem = cert_pem_start;
      config.method = HTTP_METHOD_GET;
      config.timeout_ms = 5000;

      esp_https_ota_config_t ota_config = {
          .http_config = &config,
          .http_client_init_cb = nullptr,
          .bulk_flash_erase = false,
          .partial_http_download = false,
          .max_http_request_size = 0,
          .buffer_caps = 0,
      };

      esp_err_t ret = esp_https_ota(&ota_config);
      if (ret == ESP_OK) {
        httpd_resp_send(req, nullptr, 0);
      } else {
        res_json.set("message", "OTA failed");
        auto res_str = res_json.stringify();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
      }
    } else {
      auto res_str = res_json.stringify();
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
    }

    httpd_req_async_handler_complete(req);
  }
}