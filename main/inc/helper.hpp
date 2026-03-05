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

inline uint8_t get_byte(uint16_t v, uint8_t b) {
  return get_byte(static_cast<uint32_t>(v), b);
}

inline uint8_t get_byte(float v, uint8_t b) {
  if (b >= 4)
    return 0;
  union {
    float f;
    uint32_t u;
  } x;
  x.f = v;
  return x.u >> (8 * b) & 0xff;
}

inline uint32_t u32_concat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  uint32_t u0 = b0;
  uint32_t u1 = b1;
  uint32_t u2 = b2;
  uint32_t u3 = b3;
  return (u3 << 24) | (u2 << 16) | (u1 << 8) | u0;
}

inline float f_concat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  union {
    float f;
    uint32_t u;
  } x;
  x.u = u32_concat(b0, b1, b2, b3);
  return x.f;
}

template <typename T>
inline bool get_bit(T n, int bit) {
  return (n >> bit) & 1;
}

template <typename T>
inline T set_bit(T n, int bit) {
  return n | (1 << bit);
}

template <typename T>
inline T unset_bit(T n, int bit) {
  return n & (~(1 << bit));
}