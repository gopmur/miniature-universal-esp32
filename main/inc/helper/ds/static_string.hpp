#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include "helper/ds/static_vector.hpp"

class BaseStaticString {
  protected:
  BaseStaticString(BaseStaticVector<char>* data, size_t len);
  BaseStaticVector<char>* data;

  public:
  size_t get_len();
  size_t get_size();
  char* get_data();
  bool push(char c);
  char& operator[](size_t i);
};

template <size_t N>
class StaticString : public BaseStaticString {
  private:
  static constexpr size_t BUFFER_LEN = N + 1;
  StaticVector<char, BUFFER_LEN> data;

  public:
  StaticString(char* c_str);
  StaticString();
};

template <size_t N>
StaticString<N>::StaticString()
    : BaseStaticString(static_cast<BaseStaticVector<char>*>(&data), N) {}

template <size_t N>
StaticString<N>::StaticString(char* c_str)
    : BaseStaticString(static_cast<BaseStaticVector<char>*>(&data), N) {
  size_t c_str_len = std::strlen(c_str);
  auto len = std::min(c_str_len, BUFFER_LEN - 1);
  for (int i = 0; i < len; i++) {
    data.push(c_str[i]);
  }
  data.push(0);
}