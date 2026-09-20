#include <cstdarg>
#include "esp_log.h"

#define LOGV(...) ESP_LOGV(tag, __VA_ARGS__)
#define LOGD(...) ESP_LOGD(tag, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(tag, __VA_ARGS__)
#define LOGW(...) ESP_LOGW(tag, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(tag, __VA_ARGS__)

#define MAKE_LOGGABLE(log_tag) \
  private:                     \
  static constexpr const char* tag = log_tag