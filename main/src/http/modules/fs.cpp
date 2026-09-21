#include "http/modules/fs.hpp"
#include <dirent.h>
#include <sys/dirent.h>
#include <variant>
#include "esp_http_server.h"
#include "helper/formats.hpp"
#include "jayson.hpp"

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
    resp_json.set("message", "requested path does not exist");
    return send_json(req, resp_json, HTTPD_404_NOT_FOUND);
  }
  JsonArray files_json_array;
  while (auto entry = readdir(dir)) {
    JsonObject entry_json;
    entry_json.set("name", entry->d_name);
    entry_json.set("type", get_entry_type_string(entry->d_type));
    files_json_array.append_object(&entry_json);
  }
  resp_json.set("files", &files_json_array);
  return send_json(req, resp_json);
  return ESP_OK;
}

void HttpFsModule::register_direct_uris() {
  register_uri_with_option("/ls", HTTP_PUT, put_ls);
}