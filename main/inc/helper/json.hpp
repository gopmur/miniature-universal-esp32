#pragma once

#include <variant>
#include "cJSON.h"

enum class JsonError {
  NOT_PROVIDED,
  NOT_AN_OBJECT,
  NOT_AN_ARRAY,
  NOT_A_NUMBER,
  NOT_A_STRING,
  PARSE_ERROR,
};

class JsonObject;

class Json {
  private:
  int* ref_count;
  bool owned;
  
  protected:
  cJSON* root;
  Json(cJSON* root, bool owned);
  void release();

  public:
  Json& operator=(const Json& other);
  Json(const Json& other);
  ~Json();
};

class JsonArray : public Json {
  friend JsonObject;

  private:
  JsonArray(cJSON* root, bool owned);

  public:
  JsonArray();
  static std::variant<JsonArray, JsonError> parse(char* str);

  int len();

  void append_object(JsonObject* object);

  char* stringify();
};

class JsonObject : public Json {
  friend JsonArray;

  private:
  JsonObject(cJSON* root, bool owned);

  public:
  JsonObject();
  static std::variant<JsonObject, JsonError> parse(char* str);

  bool is_empty();

  void set_error(const char* name, JsonError error);
  void set_number(const char* name, double number);
  void set_string(const char* name, const char* string);
  void set_object(const char* name, JsonObject* object);
  void add_object(const char* name);
  void set_bool(const char* name, bool boolean);
  void set_array(const char* name, JsonArray* array);

  std::variant<JsonObject, JsonError> get_object(const char* name);
  std::variant<JsonObject, JsonError> get_object(const char* name,
                                                 JsonObject* error_object);
  std::variant<double, JsonError> get_number(const char* name);
  std::variant<double, JsonError> get_number(const char* name,
                                             JsonObject* error_object);
  std::variant<char*, JsonError> get_string(const char* name);
  std::variant<char*, JsonError> get_string(const char* name,
                                            JsonObject* error_object);

  char* stringify();
};
