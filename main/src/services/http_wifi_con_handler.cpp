// #include "services/http_wifi_con_handler.hpp"
// #include <cstdlib>
// #include <variant>
// #include "esp_err.h"
// #include "esp_http_server.h"
// #include "esp_log.h"
// #include "esp_wifi.h"
// #include "esp_wifi_types_generic.h"
// #include "helper/json.hpp"

// HttpWifiConHandlerService::HttpWifiConHandlerService(int priority)
//     : Service(2, "http_wifi_con_handler") {}

// void HttpWifiConHandlerService::main() {
//   while (true) {
//     auto req_result = req_queue.receive();
//     if (!req_result.has_value()) {
//       continue;
//     }
//     auto req = req_result.value();
//     ESP_LOGI("WIFI", "WIFI CONNECTION HANDLER STARTED");

//     char* req_body = static_cast<char*>(malloc(req->content_len + 1));

//     int total = 0;
//     int remaining = req->content_len;

//     while (remaining > 0) {
//       int r = httpd_req_recv(req, req_body + total, remaining);
//       if (r <= 0) {
//         free(req_body);
//         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "recv failed");
//         httpd_req_async_handler_complete(req);
//         continue;
//       }
//       total += r;
//       remaining -= r;
//     }

//     req_body[total] = 0;
//     JsonObject error_json;

//     auto req_json_result = JsonObject::parse(req_body);
//     if (std::holds_alternative<JsonError>(req_json_result)) {
//       error_json.set("message", "parse error");
//       auto res_str = error_json.stringify();
//       httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, res_str.c_str());
//       free(req_body);
//       continue;
//     }

//     auto req_json = std::get<JsonObject>(req_json_result);

//     free(req_body);
//     auto ssid_result = req_json.get_string("ssid", &error_json);
//     auto password_result = req_json.get_string("password", &error_json);
//     wifi_config_t sta_config = {};

//     if (std::holds_alternative<char*>(ssid_result)) {
//       auto ssid = std::get<char*>(ssid_result);
//       int ssid_len = strlen(ssid);
//       if (ssid_len >= 32) {
//         error_json.set("ssid", "too long");
//       } else {
//         std::strcpy(reinterpret_cast<char*>(sta_config.sta.ssid), ssid);
//       }
//     }

//     if (std::holds_alternative<char*>(password_result)) {
//       auto password = std::get<char*>(password_result);
//       int password_len = strlen(password);
//       if (password_len >= 32) {
//         error_json.set("password", "too long");
//       } else {
//         std::strcpy(reinterpret_cast<char*>(sta_config.sta.password), password);
//       }
//     }

//     if (!error_json.is_empty()) {
//       auto res_str = error_json.stringify();
//       httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
//     }

//     else {
//       sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
//       connection_result_queue.flush();
//       ESP_ERROR_CHECK(esp_wifi_disconnect());
//       ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
//       ESP_ERROR_CHECK(esp_wifi_connect());
//       auto wifi_connection_result = connection_result_queue.receive();
//       JsonObject resp_json;
//       switch (wifi_connection_result.value()) {
//         case WifiConnectionRequestResult::OK:
//           httpd_resp_send(req, nullptr, 0);
//           break;
//         case WifiConnectionRequestResult::FAILED: {
//           resp_json.set("message", "connection failed");
//           auto resp_str = resp_json.stringify();
//           httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, resp_str.c_str());
//           break;
//         }
//         case WifiConnectionRequestResult::WRONG_SSID: {
//           resp_json.set("message", "ap not found");
//           auto resp_str = resp_json.stringify();
//           httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, resp_str.c_str());
//           break;
//         }
//         case WifiConnectionRequestResult::OTHER: {
//           resp_json.set("message", "unhandled error");
//           auto resp_str = resp_json.stringify();
//           httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, resp_str.c_str());
//           break;
//         }
//       }
//     }

//     httpd_req_async_handler_complete(req);
//   }
// }