#pragma once

#include <cstdint>
#include "esp_err.h"

#define ESP_BREAK_ON_ERROR(x)           \
  ({                                    \
    esp_err_t ret;                      \
    ret = x;                            \
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret); \
    if (ret != ERR_OK)                  \
      break;                            \
  })

#define ESP_CONTINUE_ON_ERROR(x)        \
  ({                                    \
    esp_err_t ret;                      \
    ret = x;                            \
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret); \
    if (ret != ERR_OK)                  \
      continue;                         \
  })

inline uint8_t get_byte(uint32_t v, uint8_t b) {
  if (b >= 4)
    return 0;
  return v >> (8 * b) & 0xff;
}