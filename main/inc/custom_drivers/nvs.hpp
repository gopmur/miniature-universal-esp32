#pragma once

#include <string>
#include "esp_err.h"
#include "nvs.h"
#include "system_logger.hpp"

class Nvs {
  MAKE_LOGGABLE("nvs_custom_driver");

  private:
  bool opened = false;
  std::string nvs_namespace;
  nvs_handle_t nvs;
  void handle_set_err(const char* name, esp_err_t err);

  public:
  void init(const char* name, float value);
  void init(const char* name, uint32_t value);
  void set(const char* name, float value);
  void set(const char* name, uint32_t value);
  float get_float(const char* name);
  uint32_t get_uint32(const char* name);
  Nvs(const char* nvs_namespace, nvs_open_mode_t open_mode);
  ~Nvs();
};