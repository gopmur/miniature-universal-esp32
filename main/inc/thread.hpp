#pragma once

#include <cstring>
#include "freertos/idf_additions.h"

template <typename Derived, typename Param>
struct ThreadMainParam {
  Derived* self;
  Param* param;
};

template <typename Derived, typename Param>
class AbstractThread {
  private:
  uint8_t name_counter;
  char* name_template;
  int priority;
  int stack_size;
  static void _main(ThreadMainParam<Derived, Param>* main_param);

  protected:
  AbstractThread(const char* name, int priority, int stack_size);
  void main(Param* param);

  public:
  ~AbstractThread();
  void start(Param* param);
};

template <typename Derived, typename Param>
AbstractThread<Derived, Param>::AbstractThread(const char* name,
                                               int priority,
                                               int stack_size)
    : name_counter(0), priority(priority), stack_size(stack_size) {
  this->name_template = static_cast<char*>(malloc(strlen(name) + 1));
  strcpy(this->name_template, name);
}

template <typename Derived, typename Param>
void AbstractThread<Derived, Param>::_main(
    ThreadMainParam<Derived, Param>* main_param) {
  auto self = main_param->self;
  auto param = main_param->param;
  self->main(param);
  free(main_param->self);
  free(main_param->param);
  free(main_param);
  vTaskDelete(nullptr);
}

template <typename Derived, typename Param>
void AbstractThread<Derived, Param>::start(Param* param) {
  auto main_param = static_cast<ThreadMainParam<Derived, Param>*>(
      malloc(sizeof(ThreadMainParam<Derived, Param>)));
  main_param->param = static_cast<Param*>(malloc(sizeof(Param)));
  memcpy(main_param->param, param, sizeof(Param));
  main_param->self = new Derived(*static_cast<Derived*>(this));
  char* name = static_cast<char*>(malloc(strlen(this->name_template) + 4 + 1));
  sprintf(name, "%s_%d", this->name_template, this->name_counter);
  this->name_counter++;
  xTaskCreate(reinterpret_cast<void (*)(void*)>(Derived::_main),
              name,
              this->stack_size,
              main_param,
              this->priority,
              nullptr);
  free(name);
}

template <typename Derived, typename Param>
AbstractThread<Derived, Param>::~AbstractThread() {
  free(this->name_template);
}
