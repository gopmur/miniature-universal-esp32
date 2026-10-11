#include "custom_drivers/nvs.hpp"
#include "esp_err.h"
#include "nvs.h"
#include "system_logger.hpp"

Nvs::Nvs(const char* nvs_namespace, nvs_open_mode_t open_mode) : nvs_namespace(nvs_namespace) {
  auto err = nvs_open(nvs_namespace, open_mode, &nvs);
  switch (err) {
    case ESP_FAIL:
      LOGE("failed to open namespace %s. this is probably due to corrupt nvs", nvs_namespace);
      opened = false;
      break;
    default:
      LOGE("failed to open namespace %s. %s", nvs_namespace, esp_err_to_name(err));
      opened = false;
    case ESP_OK:
      opened = true;
      break;
  }
}

Nvs::~Nvs() {
  nvs_close(nvs);
}

void Nvs::handle_set_err(const char* name, esp_err_t err) {
  if (err == ESP_OK) {
    return;
  }
  LOGE("failed to write to %s in namespace %s. %s",
       name,
       nvs_namespace.c_str(),
       esp_err_to_name(err));
}

void Nvs::init(const char* name, float value) {
  if (!opened) {
    LOGE(
        "failed to init %s with value %f in namespace %s. this happened because the opening of the "
        "namespace failed previously");
    return;
  }
  size_t stored_size = 0;
  auto err = nvs_get_blob(nvs, name, nullptr, &stored_size);
  if (err != ESP_OK) {
    LOGW("%s does not exist in namespace %s. initializing it with value %f",
         name,
         nvs_namespace.c_str(),
         value);
  } else if (stored_size != sizeof(float)) {
    LOGW(
        "size of %s (%d) in namespace %s does not match the size of float. overwriting it with "
        "value %f",
        name,
        nvs_namespace.c_str(),
        stored_size,
        value);
  }

  if (err != ESP_OK || stored_size != sizeof(float)) {
    err = nvs_set_blob(nvs, name, &value, sizeof(float));
    handle_set_err(name, err);
  }
}

void Nvs::set(const char* name, float value) {
  if (!opened) {
    LOGE(
        "failed to set %s with value %f in namespace %s. this happened because the opening of the "
        "namespace failed previously");
    return;
  }
  auto err = nvs_set_blob(nvs, name, &value, sizeof(float));
  handle_set_err(name, err);
}

float Nvs::get_float(const char* name) {
  if (!opened) {
    LOGE(
        "failed to get %s in namespace %s. this happened because the opening of the namespace "
        "failed previously");
    return 0;
  }
  size_t stored_size;
  auto err = nvs_get_blob(nvs, name, nullptr, &stored_size);
  if (err != ESP_OK) {
    LOGE("%s does not exist in namespace %s.", name, nvs_namespace.c_str());
  } else if (stored_size != sizeof(float)) {
    LOGE("size of %s (%d) in namespace %s does not match the size of float",
         name,
         nvs_namespace.c_str(),
         stored_size);
  }
  float stored_value;
  nvs_get_blob(nvs, name, &stored_value, &stored_size);
  return stored_value;
}