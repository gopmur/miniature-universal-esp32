#pragma once

#include <optional>
#include "freertos/FreeRTOS.h"

template <typename T, int N>
class Queue {
  private:
  T queue_data[N];
  StaticQueue_t queue_instance;
  QueueHandle_t queue;

  public:
  Queue();
  void send(T val, int ticks_to_wait);
  std::optional<T> receive(int ticks_to_wait);
  void flush();
  int get_remaining();
  int get_waiting();
};

template <typename T, int N>
Queue<T, N>::Queue() {
  queue = xQueueCreateStatic(N, sizeof(T), reinterpret_cast<uint8_t*>(queue_data), &queue_instance);
}

template <typename T, int N>
void Queue<T, N>::send(T val, int ticks_to_wait) {
  xQueueSend(queue, &val, ticks_to_wait);
}

template <typename T, int N>
std::optional<T> Queue<T, N>::receive(int ticks_to_wait) {
  T status;
  auto item_is_available = xQueueReceive(queue, &status, ticks_to_wait);
  if (item_is_available) {
    return status;
  }
  return std::nullopt;
}

template <typename T, int N>
void Queue<T, N>::flush() {
  xQueueReset(this->queue);
}

template <typename T, int N>
int Queue<T, N>::get_waiting() {
  return uxQueueMessagesWaiting(this->queue);
}

template <typename T, int N>
int Queue<T, N>::get_remaining() {
  return N - uxQueueMessagesWaiting(this->queue);
}