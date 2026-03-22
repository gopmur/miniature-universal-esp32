#pragma once

#include <cstdlib>
#include <cstring>
#include <vector>
#include "freertos/idf_additions.h"
#include "thread.hpp"

class ServiceThread : public Thread {
  protected:
  static void _main(ServiceThread* self);
  StaticTask_t tcb;

  virtual void main() = 0;
  ServiceThread(int stack_size, int priority, const char* name);

  public:
  static std::vector<ServiceThread*> service_list;
};

template <int STACK_SIZE>
class Service : public ServiceThread {
  private:
  StackType_t stack[STACK_SIZE];

  protected:
  Service(int priority, const char* name);

  public:
  void start();
};

template <int STACK_SIZE>
void Service<STACK_SIZE>::start() {
  service_list.push_back(this);
  this->handle = xTaskCreateStatic(reinterpret_cast<void (*)(void*)>(_main),
                                   this->name,
                                   stack_size,
                                   this,
                                   priority,
                                   stack,
                                   &tcb);
}

template <int STACK_SIZE>
Service<STACK_SIZE>::Service(int priority, const char* name)
    : ServiceThread(STACK_SIZE, priority, name) {}