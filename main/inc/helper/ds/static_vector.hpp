#pragma once

#include <cstddef>
#include <optional>

template <typename T>
class BaseStaticVector {
  protected:
  BaseStaticVector(T* data, size_t size);
  const size_t SIZE;
  size_t len = 0;
  T* data;

  public:
  bool push(T v);
  T pop();
  bool remove(size_t len);
  bool remove_if(bool (*f)(const T&));
  std::optional<size_t> index_of(bool (*f)(const T&));
  T& operator[](size_t i);
  void flush();
  size_t get_len();
  size_t get_size();
  T* get_data();
};

template <typename T>
BaseStaticVector<T>::BaseStaticVector(T* data, size_t size) : SIZE(size), data(data) {
  
}
template <typename T>
size_t BaseStaticVector<T>::get_size() {
  return SIZE;
}

template <typename T>
T* BaseStaticVector<T>::get_data() {
  return this->data;
}

template <typename T>
T& BaseStaticVector<T>::operator[](size_t i) {
  if (i >= len) {
    // return std::nullopt;
  }
  return data[i];
}

template <typename T>
bool BaseStaticVector<T>::push(T v) {
  if (len >= SIZE) {
    return false;
  }
  data[len] = v;
  len++;
  return true;
}

template <typename T>
bool BaseStaticVector<T>::remove(size_t i) {
  if (i >= len) {
    return false;
  }
  for (int j = i; j < len - 1; j++) {
    data[j] = data[j + 1];
  }
  len--;
  return true;
}

template <typename T>
T BaseStaticVector<T>::pop() {
  if (len <= 0) {
    // return std::nullopt;
    return 0;
  }
  len--;
  return data[len];
}

template <typename T>
std::optional<size_t> BaseStaticVector<T>::index_of(bool (*f)(const T&)) {
  for (int i = 0; i < len; i++) {
    if (f(data[i])) {
      return i;
    }
  }
  return std::nullopt;
}

template <typename T>
bool BaseStaticVector<T>::remove_if(bool (*f)(const T&)) {
  auto i = index_of(f);
  if (!i.has_value()) {
    return false;
  }
  return remove(i.value());
}

template <typename T>
size_t BaseStaticVector<T>::get_len() {
  return len;
}

template <typename T>
void BaseStaticVector<T>::flush() {
  len = 0;
}

template <typename T, size_t N>
class StaticVector : public BaseStaticVector<T> {
  private:
  T data[N];

  public:
  StaticVector();
};

template <typename T, size_t N>
StaticVector<T, N>::StaticVector() : BaseStaticVector<T>(data, N) {}
