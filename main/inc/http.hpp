#pragma once

#define DEFINE_FILE_GET_HANDLER(file_name, type)                               \
  static esp_err_t file_name##_get(httpd_req_t* req) {                         \
    extern const char file_name##_start[] asm("_binary_" #file_name "_start"); \
    extern const char file_name##_file_end[] asm("_binary_" #file_name         \
                                                 "_end");                      \
    const size_t size = file_name##_file_end - file_name##_start;              \
    esp_err_t res = httpd_resp_set_type(req, type);                            \
    if (res) {                                                                 \
      return res;                                                              \
    }                                                                          \
    return httpd_resp_send(req, file_name##_start, size);                      \
  }

#define REGISTER_FILE_URI(server_instance, file_name, file_uri)    \
  {                                                                \
    static const httpd_uri_t file_name##_uri = {                   \
        .uri = file_uri,                                           \
        .method = HTTP_GET,                                        \
        .handler = file_name##_get,                                \
        .user_ctx = NULL,                                          \
    };                                                             \
    httpd_register_uri_handler(server_instance, &file_name##_uri); \
  }