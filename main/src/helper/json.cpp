#include "helper/json.hpp"
#include <variant>

#include "cJSON.h"

Json::Json(cJSON* root) : owned(true), root(root) {}
Json::Json(cJSON* root, bool owned) : owned(owned), root(root) {}
Json::Json(char* str) : owned(true) {
  this->root = cJSON_Parse(str);
}
Json::Json() {
  this->root = cJSON_CreateObject();
}
Json::~Json() {
  if (owned && root) {
    cJSON_Delete(this->root);
  }
}

void Json::set_string(const char* name, const char* string) {
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item && cJSON_IsString(item)) {
    cJSON_SetValuestring(item, string);
  } else if (item) {
    cJSON_ReplaceItemInObject(this->root, name, cJSON_CreateString(string));
  } else {
    cJSON_AddStringToObject(this->root, name, string);
  }
}

void Json::set_number(const char* name, double number) {
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item && cJSON_IsNumber(item)) {
    cJSON_SetNumberValue(item, number);
  } else if (item) {
    cJSON_ReplaceItemInObject(this->root, name, cJSON_CreateNumber(number));
  } else {
    cJSON_AddNumberToObject(this->root, name, number);
  }
}

void Json::set_object(const char* name, Json* object) {
  cJSON* copy = cJSON_Duplicate(object->root, 1);
  if (!copy)
    return;
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item) {
    cJSON_ReplaceItemInObject(this->root, name, copy);
  } else {
    cJSON_AddItemToObject(this->root, name, copy);
  }
}

void Json::set_bool(const char* name, bool boolean) {
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item && cJSON_IsBool(item)) {
    cJSON_SetBoolValue(item, boolean);
  } else if (item) {
    cJSON_ReplaceItemInObject(this->root, name, cJSON_CreateBool(boolean));
  } else {
    cJSON_AddBoolToObject(this->root, name, boolean);
  }
}

void Json::set_error(const char* name, JsonError error) {
  const char* error_message;
  switch (error) {
    case JsonError::NOT_PROVIDED:
      error_message = "not provided";
      break;
    case JsonError::NOT_A_NUMBER:
      error_message = "not a number";
      break;
    case JsonError::NOT_A_STRING:
      error_message = "not a string";
      break;
    case JsonError::NOT_AN_OBJECT:
      error_message = "not an object";
      break;
  }
  this->set_string(name, error_message);
}

std::variant<Json, JsonError> Json::get_object(const char* name) {
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item == nullptr) {
    return JsonError::NOT_PROVIDED;
  } else if (!cJSON_IsObject(item)) {
    return JsonError::NOT_AN_OBJECT;
  } else {
    return Json(item, false);
  }
}

std::variant<Json, JsonError> Json::get_object(const char* name,
                                               Json* error_object) {
  auto child_object = this->get_object(name);
  if (std::holds_alternative<JsonError>(child_object)) {
    error_object->set_error(name, std::get<JsonError>(child_object));
  };
  return child_object;
};

std::variant<double, JsonError> Json::get_number(const char* name) {
  auto item = cJSON_GetObjectItem(this->root, name);
  if (item == nullptr) {
    return JsonError::NOT_PROVIDED;
  } else if (!cJSON_IsNumber(item)) {
    return JsonError::NOT_A_NUMBER;
  } else {
    return cJSON_GetNumberValue(item);
  }
}

std::variant<double, JsonError> Json::get_number(const char* name,
                                                 Json* error_object) {
  auto number = this->get_number(name);
  if (std::holds_alternative<JsonError>(number)) {
    error_object->set_error(name, std::get<JsonError>(number));
  }
  return number;
}

Json::Json(const Json& other) : owned(true) {
  if (other.root) {
    root = cJSON_Duplicate(other.root, 1);
  } else {
    root = nullptr;
  }
}

Json& Json::operator=(const Json& other) {
  if (this == &other) {
    return *this;
  }

  if (this->owned && this->root) {
    cJSON_Delete(this->root);
  }

  if (other.root) {
    this->root = cJSON_Duplicate(other.root, 1);
    this->owned = true;
  } else {
    this->root = nullptr;
    this->owned = true;
  }
  return *this;
}

bool Json::is_empty() {
  return cJSON_IsObject(this->root) && this->root->child == nullptr;
}

char* Json::stringify() {
  return cJSON_PrintUnformatted(this->root);
}