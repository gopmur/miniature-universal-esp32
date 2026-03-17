#pragma once

#include <variant>
#include "cJSON.h"

enum class JsonError {
  NOT_PROVIDED,
  NOT_AN_OBJECT,
  NOT_A_NUMBER,
  NOT_A_STRING,
};

class Json {
  private:
  bool owned;
  cJSON* root;
  void set_error(const char* name, JsonError error);
  Json(cJSON* root, bool owned);

  public:
  Json& operator=(const Json& other);
  Json(const Json& other);
  Json(cJSON* root);
  Json(char* str);
  Json();
  ~Json();

  bool is_empty();

  void set_number(const char* name, double number);
  void set_string(const char* name, const char* string);
  void set_object(const char* name, Json* object);
  void set_bool(const char* name, bool boolean);

  std::variant<Json, JsonError> get_object(const char* name);
  std::variant<Json, JsonError> get_object(const char* name,
                                           Json* error_object);
  std::variant<double, JsonError> get_number(const char* name);
  std::variant<double, JsonError> get_number(const char* name,
                                             Json* error_object);
  std::variant<char*, JsonError> get_string(const char* name);
  std::variant<char*, JsonError> get_string(const char* name,
                                            Json* error_object);

  char* stringify();
};
