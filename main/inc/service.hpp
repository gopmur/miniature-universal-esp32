#pragma once

#include <cstdlib>
#include <cstring>
#include "freertos/idf_additions.h"
#include "thread.hpp"

class _AbstractService : public _AbstractThread {
  protected:
  static void _main(_AbstractService* self);
  StaticTask_t tcb;
  char* name;

  virtual void main() = 0;
  _AbstractService(int stack_size, int priority, const char* name);
};

template <int STACK_SIZE>
class AbstractService : public _AbstractService {
  private:
  StackType_t stack[STACK_SIZE]; /**< Stack memory for the task */

  protected:
  AbstractService(int priority, const char* name);

  public:
  void start();
};

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::start() {
  this->handle =
      xTaskCreateStatic(reinterpret_cast<void (*)(void*)>(_main), this->name, stack_size, this, priority, stack, &tcb);
}

template <int STACK_SIZE>
AbstractService<STACK_SIZE>::AbstractService(int priority, const char* name)
    : _AbstractService(STACK_SIZE, priority, name) {}