#pragma once

#include "freertos/idf_additions.h"
#include <optional>

template <typename T>
class Queue {
 private:
  QueueHandle_t queue;

 public:
  Queue(int len);
  void send(T val, int ticks_to_wait);
  std::optional<T> receive(int ticks_to_wait);
};

template <typename T>
Queue<T>::Queue(int len) {
  queue = xQueueCreate(len, sizeof(T));
}

template <typename T>
void Queue<T>::send(T val, int ticks_to_wait) {
  xQueueSend(queue, &val, ticks_to_wait);
}

template <typename T>
std::optional<T> Queue<T>::receive(int ticks_to_wait) {
  T status;
  auto item_is_available = xQueueReceive(queue, &status, ticks_to_wait);
  if (item_is_available) {
    return status;
  }
  return std::nullopt;
}
