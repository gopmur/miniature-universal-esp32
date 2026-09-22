#include "system_logger.hpp"
#include <sys/unistd.h>
#include <atomic>
#include <cstdio>
#include <ctime>
#include <format>
#include <string>
#include <utility>
#include "esp_http_server.h"
#include "http.hpp"
#include "tasks/ws.hpp"

extern WebSocketTask ws_task;
extern HttpServer http_server;

Mutex SystemLogger::log_file_mutex(true);
FILE* SystemLogger::log_file;

int SystemLogger::log_vprintf(const char* fmt, va_list args) {
  va_list file_args;
  va_list ws_args;

  va_copy(file_args, args);
  log_file_mutex.take();
  if (log_file) {
    vfprintf(log_file, fmt, file_args);
    fsync(fileno(log_file));
    fflush(log_file);
  }
  log_file_mutex.give();
  va_end(file_args);

  auto ws_connections = ws_task.get_connections();

  for (auto connection : ws_connections) {
    va_copy(ws_args, args);
    size_t len = vsnprintf(nullptr, 0, fmt, args);
    if (connection.stream == WsStream::SYS_LOG) {
      auto buffer = new std::string;
      buffer->resize(len);
      vsprintf(buffer->data(), fmt, ws_args);
      ws_task.sys_log_queue.send(std::make_pair(connection.fd, buffer));
    }
    va_end(ws_args);
  }

  int ret = vprintf(fmt, args);

  return ret;
}

void SystemLogger::close_log_file() {
  log_file_mutex.take();
  int status = fclose(log_file);
  if (status != 0) {
    LOGE("failed to close log file");
  }
  log_file = nullptr;
  log_file_mutex.give();
}

void SystemLogger::init() {
  log_file = fopen("/sd/sys.log", "w");
  esp_log_set_vprintf(log_vprintf);
};

void SystemLogger::update_log_file_name() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  LOGI("updating the log file's name");
  std::string log_file_name = std::format("/sd/{}-{}-{}-{}-{}-{}.log",
                                          timeinfo.tm_year + 1900,
                                          timeinfo.tm_mon + 1,
                                          timeinfo.tm_mday,
                                          timeinfo.tm_hour,
                                          timeinfo.tm_min,
                                          timeinfo.tm_sec);
  log_file_mutex.take();
  int status = fclose(log_file);
  log_file = nullptr;
  if (status != 0) {
    LOGE("closing log file failed while trying to change its name");
    goto cleanup;
  }
  LOGI("old log file closed. trying to rename it");
  status = rename("/sd/sys.log", log_file_name.c_str());
  if (status != 0) {
    LOGE("log file renaming failed trying to reopen old file");
    log_file = fopen("/sd/sys.log", "a");
    if (log_file == nullptr) {
      LOGE("log file reopening failed");
      goto cleanup;
    }
  }
  LOGI("log file renaming was successful. trying to reopen it");
  log_file = fopen(log_file_name.c_str(), "a");
  if (log_file == nullptr) {
    LOGE("could not open new log file");
    goto cleanup;
  }
  LOGI("log file reopen was successful");
cleanup:
  log_file_mutex.give();
};