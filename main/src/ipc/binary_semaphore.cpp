#include "ipc/binary_semaphore.hpp"
#include "freertos/idf_additions.h"

BinarySemaphore::BinarySemaphore() : BinarySemaphore(false) {}

BinarySemaphore::BinarySemaphore(bool recursive) {
  semaphore = xSemaphoreCreateBinaryStatic(&semaphore_buffer);
}

void BinarySemaphore::take() {
  xSemaphoreTake(this->semaphore, portMAX_DELAY);
}

void BinarySemaphore::give() {
  xSemaphoreGive(this->semaphore);
}
