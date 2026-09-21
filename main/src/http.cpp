#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <variant>

#include "http.hpp"

#include "config.hpp"
#include "custom_drivers/motor.hpp"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "jayson.hpp"
#include "http_assets.hpp"
#include "http_parser.h"

#include "jaythread/sync.hpp"
#include "tasks/control.hpp"
#include "http/helper.hpp"
#include "http/module.hpp"
#include "tasks/logger.hpp"
#include "tasks/motor.hpp"
#include "tasks/wifi_con_handler.hpp"
#include "tasks/ws.hpp"
// #include "threads/scan_wifis.hpp"

// extern WebSocketTask* ws_task;
// extern ControlTask* control_task;
// extern MotorTask* motor_task;
// extern LoggerTask* logger_task;
// extern AbstractMotorDriver* left_motor;
// extern AbstractMotorDriver* right_motor;
// ScanWifisThread scan_wifis_thread;



// esp_err_t null_request_handler(httpd_req_t* req) {
//   set_header(req);
//   httpd_resp_send(req, nullptr, 0);
//   return ESP_OK;
// }


// esp_err_t start_log(httpd_req_t* req) {
//   set_header(req);
//   logger_task->start_new_log();
//   httpd_resp_send(req, nullptr, 0);
//   return ESP_OK;
// }

// esp_err_t stop_log(httpd_req_t* req) {
//   set_header(req);
//   logger_task->stop_log();
//   httpd_resp_send(req, nullptr, 0);
//   return ESP_OK;
// }

// // void set_rtc_time(JsonObject* time_json, JsonObject* time_error_json) {
// //   auto hours_item = time_json->get_number("hours", time_error_json);
// //   auto minutes_item = time_json->get_number("minutes", time_error_json);
// //   auto seconds_item = time_json->get_number("seconds", time_error_json);
// //   if (std::holds_alternative<JsonError>(hours_item) ||
// //       std::holds_alternative<JsonError>(minutes_item) ||
// //       std::holds_alternative<JsonError>(seconds_item)) {
// //     return;
// //   }
// //   uint8_t hours = std::get<double>(hours_item);
// //   uint8_t minutes = std::get<double>(minutes_item);
// //   uint8_t seconds = std::get<double>(seconds_item);
// //   std::array<uint8_t, 4> time_data = {hours, minutes, seconds, 0};
// //   write_address(SspAddress::RTC_TIME, time_data);
// // }

// // void set_rtc_date(JsonObject* date_json, JsonObject* date_error_json) {
// //   auto year_item = date_json->get_number("year", date_error_json);
// //   auto month_item = date_json->get_number("month", date_error_json);
// //   auto day_item = date_json->get_number("day", date_error_json);
// //   if (std::holds_alternative<JsonError>(year_item) ||
// //       std::holds_alternative<JsonError>(month_item) ||
// //       std::holds_alternative<JsonError>(day_item)) {
// //     return;
// //   }
// //   uint16_t year = std::get<double>(year_item);
// //   uint8_t month = std::get<double>(month_item);
// //   uint8_t day = std::get<double>(day_item);
// //   std::array<uint8_t, 4> date_data = {get_byte(year, 1), get_byte(year, 0), month, day};
// //   write_address(SspAddress::RTC_DATE, date_data);
// // }

// // esp_err_t set_rtc(httpd_req_t* req) {
// //   set_header(req);
// //   char* req_body = static_cast<char*>(malloc(req->content_len + 1));
// //   int received = httpd_req_recv(req, req_body, req->content_len);
// //   req_body[received] = 0;

// //   JsonObject res_json;
// //   auto req_json_result = JsonObject::parse(req_body);
// //   free(req_body);

// //   if (std::holds_alternative<JsonError>(req_json_result)) {
// //     res_json.set("message", "parse error");
// //     auto res_str = res_json.stringify();
// //     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
// //     return ESP_OK;
// //   }

// //   auto req_json = std::get<JsonObject>(req_json_result);

// //   auto time_item = req_json.get_object("time", &res_json);
// //   auto date_item = req_json.get_object("date", &res_json);

// //   if (std::holds_alternative<JsonObject>(time_item)) {
// //     JsonObject time_json = std::get<JsonObject>(time_item);
// //     JsonObject time_error_json;
// //     set_rtc_time(&time_json, &time_error_json);
// //     if (!time_error_json.is_empty()) {
// //       res_json.set("time", &time_error_json);
// //     }
// //   }

// //   if (std::holds_alternative<JsonObject>(date_item)) {
// //     JsonObject date_json = std::get<JsonObject>(date_item);
// //     JsonObject date_error_json;
// //     set_rtc_date(&date_json, &date_error_json);
// //     if (!date_error_json.is_empty()) {
// //       res_json.set("date", &date_error_json);
// //     }
// //   }

// //   auto res_str = res_json.stringify();
// //   if (res_json.is_empty()) {
// //     httpd_resp_send(req, res_str.c_str(), HTTPD_RESP_USE_STRLEN);
// //   } else {
// //     httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, res_str.c_str());
// //   }

// //   return ESP_OK;
// // }

// void allow_cors(httpd_req_t* req) {
//   httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//   httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, PUT, POST, OPTIONS");
//   httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
// }

// void set_close_connection(httpd_req_t* req) {
//   httpd_resp_set_hdr(req, "Connection", "close");
// }
// void set_type_json(httpd_req_t* req) {
//   httpd_resp_set_type(req, "application/json");
// }
// void set_header(httpd_req_t* req) {
//   allow_cors(req);
//   set_close_connection(req);
//   set_type_json(req);
// }

// esp_err_t restart_handler(httpd_req_t* req) {
//   set_header(req);
//   ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), "LOG_TAG", "Restart failed");
//   Sync::sleep(10);
//   esp_restart();
//   return ESP_OK;
// }

// // esp_err_t restart_stm32_handler(httpd_req_t* req) {
// //   set_header(req);
// //   send_command(SspCommand::RESTART);
// //   uart_flush(config::stm_uart::port);
// //   ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), "LOG_TAG", "Restart
// // failed ");
// //   return ESP_OK;
// // }
// // esp_err_t restart32_handler(httpd_req_t* req) {
// //   set_header(req);
// //   ESP_RETURN_ON_ERROR(httpd_resp_send(req, nullptr, 0), "LOG_TAG", "Restart
// // failed ");
// //   vTaskDelay(10);
// //   esp_restart();
// //   return ESP_OK;
// // }

// //     esp_err_t
// //     options_handler(httpd_req_t* req) {
// //   set_header(req);
// //   httpd_resp_send(req, NULL, 0);
// //   return ESP_OK;
// // }



// // // ! this needs to be async
// // esp_err_t check_for_update_handler(httpd_req_t* req) {
// //   set_header(req);
// //   esp_http_client_config_t config{};
// //   config.url = "http://192.168.4.2:3000/latest-version";
// //   config.method = HTTP_METHOD_GET;
// //   config.timeout_ms = 5000;

// //   esp_http_client_handle_t client = esp_http_client_init(&config);

// //   uint32_t status_code = 200;
// //   int resp_len;
// //   std::string resp_str;
// //   char* resp_buffer;
// //   std::string status_code_str;
// //   JsonObject error_json;

// //   auto result = esp_http_client_open(client, 0);
// //   if (result != ESP_OK) {
// //     status_code = 500;
// //     error_json.set("message", "could not reach server");
// //     resp_str = error_json.stringify();
// //     goto send;
// //   }

// //   result = esp_http_client_fetch_headers(client);
// //   resp_len = esp_http_client_get_content_length(client);
// //   resp_buffer = new char[resp_len + 1];
// //   esp_http_client_read(client, resp_buffer, resp_len);
// //   resp_buffer[resp_len] = 0;
// //   resp_str = std::string(resp_buffer);
// //   delete[] resp_buffer;
// //   status_code = esp_http_client_get_status_code(client);

// // send:
// //   if (!error_json.is_empty()) {
// //     resp_str = error_json.stringify();
// //   }
// //   status_code_str = std::format("{}", status_code);
// //   httpd_resp_send_custom_err(req, status_code_str.c_str(), resp_str.c_str());
// //   esp_http_client_close(client);
// //   esp_http_client_cleanup(client);

// //   return ESP_OK;
// // }
// // esp_err_t get_version_handler(httpd_req_t* req) {
// //   set_header(req);
// //   JsonObject resp_json;
// //   resp_json.set("api", API_VERSION);
// //   resp_json.set("core", CORE_VERSION);
// //   auto resp_str = resp_json.stringify();
// //   httpd_resp_send(req, resp_str.c_str(), HTTPD_RESP_USE_STRLEN);
// //   return ESP_OK;
// // }

// // esp_err_t update_handler(httpd_req_t* req) {
// //   ota_busy = true;
// //   set_header(req);
// //   httpd_req_t* async_req;
// //   httpd_req_async_handler_begin(req, &async_req);
// //   http_ota_handler_service.queue.send(async_req);
// //   return ESP_OK;
// // }

// // esp_err_t get_ota_status(httpd_req_t* req) {
// //   set_header(req);
// //   JsonObject resp_json;
// //   resp_json.set("busy", ota_busy);
// //   auto resp_str = resp_json.stringify();
// //   httpd_resp_send(req, resp_str.c_str(), HTTPD_RESP_USE_STRLEN);
// //   return ESP_OK;
// // }




// esp_err_t register_dynamic_endpoints() {
//   this->register_http_uri("/api/null", HTTP_GET, null_request_handler);
//   // this->register_http_uri("/api/version", HTTP_GET, get_version_handler);
//   // this->register_http_uri("/api/update/status", HTTP_GET, get_ota_status);
//   // this->register_http_uri("/api/update/check", HTTP_GET,
//   // check_for_update_handler);

//   this->register_http_uri("/api/motors/zero_pos", HTTP_GET, motor_zero_pos);
//   this->register_http_uri("/api/log/stop", HTTP_GET, stop_log);
//   this->register_http_uri("/api/log/start", HTTP_GET, start_log);


//   return ESP_OK;
// }

HttpServer::HttpServer(HttpModule* root_module) : root_module(root_module){};

void HttpServer::start() {
  httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
  http_config.max_uri_handlers = 128;
  ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
  http_server_register_assets(server_instance);
  root_module->register_uris(server_instance);
  // ESP_ERROR_CHECK(register_dynamic_endpoints());
}
