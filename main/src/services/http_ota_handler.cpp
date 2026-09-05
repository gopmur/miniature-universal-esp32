// #include "services/http_ota_handler.hpp"
// #include <variant>
// #include "context/ota_progress.hpp"
// #include "context/services/ws.hpp"
// #include "esp_http_client.h"
// #include "esp_http_server.h"
// #include "esp_https_ota.h"
// #include "esp_log.h"
// #include "helper/json.hpp"

// extern const char cert_pem_start[] asm("_binary_cert_pem_start");

// HttpOtaHandlerService::HttpOtaHandlerService(int priority)
//     : Service(priority, "http_ota_handler") {}

// void HttpOtaHandlerService::main() {
//   while (true) {
//     auto req_result = queue.receive();
//     if (!req_result.has_value()) {
//       continue;
//     }
//     auto req = req_result.value();

//     char* req_body = static_cast<char*>(malloc(req->content_len + 1));
//     httpd_req_recv(req, req_body, req->content_len);
//     auto req_json_result = JsonObject::parse(req_body);
//     free(req_body);

//     JsonObject res_json;
//     if (std::holds_alternative<JsonError>(req_json_result)) {
//       res_json.set("message", "parse error");
//       auto res_str = res_json.stringify();
//       httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
//       httpd_req_async_handler_complete(req);
//       continue;
//     }

//     auto req_json = std::get<JsonObject>(req_json_result);
//     auto core_version_result = req_json.get_string("core", &res_json);
//     auto api_version_result = req_json.get_string("api", &res_json);

//     if (std::holds_alternative<char*>(core_version_result) &&
//         std::holds_alternative<char*>(api_version_result)) {
//       auto core_version = std::get<char*>(core_version_result);
//       auto api_version = std::get<char*>(api_version_result);

//       char update_file_url[128];
//       snprintf(update_file_url,
//                64,
//                "https://19u2.168.4.2:3001/firmware/core-%s-api-%s.bin",
//                core_version,
//                api_version);

//       ESP_LOGI("OTA", "%s", update_file_url);

//       esp_http_client_config_t config{};
//       config.url = update_file_url;
//       config.cert_pem = cert_pem_start;
//       config.method = HTTP_METHOD_GET;
//       config.timeout_ms = 5000;

//       esp_https_ota_config_t ota_config = {
//           .http_config = &config,
//           .http_client_init_cb = nullptr,
//           .bulk_flash_erase = false,
//           .buffer_caps = 0,
//           .ota_resumption = true,
//       };

//       esp_https_ota_handle_t ota_handle;
//       esp_err_t err;
//       err = esp_https_ota_begin(&ota_config, &ota_handle);
//       if (err != ESP_OK) {
//         res_json.set("message", "ota begin failed");
//         auto res_str = res_json.stringify();
//         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
//         goto cleanup;
//       }

//       bool first_iteration = true;
//       while (true) {
//         err = esp_https_ota_perform(ota_handle);
//         int downloaded = esp_https_ota_get_image_len_read(ota_handle);
//         int total = esp_https_ota_get_image_size(ota_handle);

//         if (total > 0) {
//           ota_total = total;
//           ota_progress = downloaded;
//           if (first_iteration) {
//             ws_service.enable_stream(WsStream::OTA_PROGRESS);
//             first_iteration = false;
//           }
//         }

//         if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
//           break;
//         }
//       }
//       ws_service.disable_stream(WsStream::OTA_PROGRESS);

//       JsonObject ws_json;
//       JsonObject ota_ws_result_json;

//       if (err != ESP_OK) {
//         ota_ws_result_json.set("ok", false);
//         ws_json.set("ota", &ota_ws_result_json);
//         auto ws_json_str = ws_json.stringify();
//         ws_service.send_to_connections(ws_json_str.c_str());

//         esp_https_ota_abort(ota_handle);
//         res_json.set("message", "ota failed");
//         auto res_str = res_json.stringify();
//         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
//         goto cleanup;
//       }

//       ota_ws_result_json.set("ok", true);
//       ws_json.set("ota", &ota_ws_result_json);
//       auto ws_json_str = ws_json.stringify();
//       ws_service.send_to_connections(ws_json_str.c_str());
//       esp_https_ota_finish(ota_handle);
//       httpd_resp_send(req, nullptr, 0);
//       goto cleanup;

//     } else {
//       auto res_str = res_json.stringify();
//       httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
//     }

//   cleanup:
//     httpd_req_async_handler_complete(req);
//     ota_busy = false;
//   }
// }