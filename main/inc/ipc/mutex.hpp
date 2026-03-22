#pragma once

#include "freertos/idf_additions.h"

class Mutex {
  private:
  QueueHandle_t mutex;

  public:
  Mutex();

  void take();
  void give();
};