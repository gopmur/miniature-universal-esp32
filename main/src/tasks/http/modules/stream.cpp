#include "tasks/http/modules/stream.hpp"
#include "tasks/ws.hpp"

extern WebSocketTask* ws_task;

esp_err_t HttpStreamModule::ws_imu(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_imu_post_handshake(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_task->start_sending(client_fd, WsStream::IMU_DATA);
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_motor(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_motor_post_handshake(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_task->start_sending(client_fd, WsStream::MOTOR_DATA);
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_tasks(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_tasks_post_handshake(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_task->start_sending(client_fd, WsStream::ESP_TASK_DATA);
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_ota(httpd_req_t* req) {
  return ESP_OK;
}

esp_err_t HttpStreamModule::ws_ota_post_handshake(httpd_req_t* req) {
  auto client_fd = httpd_req_to_sockfd(req);
  ws_task->start_sending(client_fd, WsStream::OTA_PROGRESS);
  return ESP_OK;
}