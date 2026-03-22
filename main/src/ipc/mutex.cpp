#include "ipc/mutex.hpp"

Mutex::Mutex() {
  this->mutex = xSemaphoreCreateMutex();
}

void Mutex::take() {
  xSemaphoreTake(this->mutex, portMAX_DELAY);
}

void Mutex::give() {
  xSemaphoreGive(this->mutex);
}
