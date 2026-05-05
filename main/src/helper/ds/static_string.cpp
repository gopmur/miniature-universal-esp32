#include "helper/ds/static_string.hpp"
#include "helper/ds/static_vector.hpp"

BaseStaticString::BaseStaticString(BaseStaticVector<char>* data, size_t buffer_size) : data(data) {}

size_t BaseStaticString::get_len() {
  return data->get_len();
}

size_t BaseStaticString::get_size() {
  return data->get_size() - 1;
}

char* BaseStaticString::get_data() {
  return data->get_data();
}

char& BaseStaticString::operator[](size_t i) {
  return (*data)[i];
}

bool BaseStaticString::push(char c) {
  if (get_len() >= get_size()) {
    return false;
  }
  (*data)[get_len() + 1] = 0;
  return data->push(c);
}

