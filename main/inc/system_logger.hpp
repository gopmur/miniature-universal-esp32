#pragma once

#include <cstdarg>
#include "esp_log.h"
#include "jaythread/ipc/mutex.hpp"

#define LOGV(...) ESP_LOGV(tag, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(tag, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(tag, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(tag, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(tag, __VA_ARGS__)

#define MAKE_LOGGABLE(log_tag) \
  private:                     \
  static constexpr const char* tag = log_tag

class SystemLogger {
  MAKE_LOGGABLE("system_logger");

  private:
  static Mutex log_file_mutex;
  static FILE* log_file;
  static int log_vprintf(const char* fmt, va_list args);

  public:
  static void update_log_file_name();
  static void init();
};