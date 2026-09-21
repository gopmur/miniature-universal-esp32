#include "http/modules/fs.hpp"
#include <dirent.h>
#include <sys/dirent.h>
#include <cstdio>
#include <variant>
#include "esp_http_server.h"
#include "helper/formats.hpp"
#include "jayson.hpp"

HttpCatThread HttpFsModule::http_cat_thread;

void HttpCatThread::main(httpd_req_t** req_p) {
  auto req = *req_p;
  auto req_data = new char[req->content_len];
  httpd_req_recv(req, req_data, req->content_len);
  JsonObject resp_json;
  auto req_json_result = JsonObject::parse(req_data);
  std::string absolute_path;
  FILE* file = nullptr;
  uint8_t* buffer = nullptr;
  JsonObject req_json;
  std::variant<char*, JsonError> path_result;
  char* path = nullptr;
  const size_t buffer_size = 1 << 15;
  if (std::holds_alternative<JsonError>(req_json_result)) {
    resp_json.set("message", "parse error");
    HttpFsModule::send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }
  req_json = std::get<JsonObject>(req_json_result);
  path_result = req_json.get_string("path", &resp_json);
  if (std::holds_alternative<JsonError>(path_result)) {
    HttpFsModule::send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }
  path = std::get<char*>(path_result);
  if (strlen(path) == 0) {
    resp_json.set("path", "cannot be empty");
    HttpFsModule::send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }
  if (path[0] != '/') {
    resp_json.set("path", "should start with /");
    HttpFsModule::send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
    goto cleanup;
  }
  absolute_path = std::string("/sd") + path;
  file = fopen(absolute_path.c_str(), "r");
  if (!file) {
    resp_json.set("path", "does not exist");
    HttpFsModule::send_json(req, resp_json, HTTPD_404_NOT_FOUND);
    goto cleanup;
  }
  buffer = new uint8_t[buffer_size];
  size_t size;
  do {
    size = fread(buffer, 1, buffer_size, file);
    httpd_resp_send_chunk(req, reinterpret_cast<char*>(buffer), size);
  } while (size > 0);
  fclose(file);
  httpd_resp_send_chunk(req, NULL, 0);

cleanup:
  if (file) {
    fclose(file);
  }
  if (buffer) {
    delete[] buffer;
  }
  httpd_req_async_handler_complete(req);
}

esp_err_t HttpFsModule::put_ls(httpd_req_t* req) {
  set_header(req);
  auto data = new char[req->content_len];
  httpd_req_recv(req, data, req->content_len);
  JsonObject resp_json;
  auto req_json_result = JsonObject::parse(data);
  if (std::holds_alternative<JsonError>(req_json_result)) {
    resp_json.set("message", "parse error");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto req_json = std::get<JsonObject>(req_json_result);
  auto path_result = req_json.get_string("path", &resp_json);
  if (std::holds_alternative<JsonError>(path_result)) {
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  auto path = std::get<char*>(path_result);
  if (strlen(path) == 0) {
    resp_json.set("path", "cannot be empty");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  if (path[0] != '/') {
    resp_json.set("path", "should start with /");
    return send_json(req, resp_json, HTTPD_400_BAD_REQUEST);
  }
  std::string absolute_path = std::string("/sd") + path;
  DIR* dir = opendir(absolute_path.c_str());
  if (dir == nullptr) {
    resp_json.set("message", "does not exist");
    return send_json(req, resp_json, HTTPD_404_NOT_FOUND);
  }
  JsonArray files_json_array;
  while (auto entry = readdir(dir)) {
    JsonObject entry_json;
    entry_json.set("name", entry->d_name);
    entry_json.set("type", get_entry_type_string(entry->d_type));
    files_json_array.append_object(&entry_json);
  }
  closedir(dir);
  resp_json.set("files", &files_json_array);
  return send_json(req, resp_json);
  return ESP_OK;
}

esp_err_t HttpFsModule::put_cat(httpd_req_t* req) {
  httpd_req_t* async_req;
  httpd_req_async_handler_begin(req, &async_req);
  http_cat_thread.start("http_cat", 2, 4096, async_req);
  return ESP_OK;
}

void HttpFsModule::register_direct_uris() {
  register_uri_with_option("/ls", HTTP_PUT, put_ls);
  register_uri_with_option("/cat", HTTP_PUT, put_cat);
}