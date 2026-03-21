#pragma once

#include <cstring>
#include "esp_log.h"
#include "freertos/idf_additions.h"

class _AbstractThread {
  private:
  uint32_t last_total_runtime = 0;
  uint32_t last_service_runtime = 0;

  protected:
  char* name;
  int priority;
  TaskHandle_t handle;
  void wait_for_notification();
  void wait_for_notification(int ticks_to_wait);
  _AbstractThread(const char* name, int priority, int stack_size);
  _AbstractThread(_AbstractThread& other);

  public:
  const int stack_size;

  void suspend();
  void resume();
  void resume_from_isr();
  void notify();
  bool notify_from_isr();
  float calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime);
  TaskHandle_t get_handle();
  ~_AbstractThread();
};

template <typename Derived, typename Param>
class AbstractThread;

template <typename Derived, typename Param>
struct ThreadMainParam {
  AbstractThread<Derived, Param>* self;
  Param* param;
};

template <typename Derived, typename Param>
class AbstractThread : public _AbstractThread {
  private:
  static void _main(ThreadMainParam<Derived, Param>* main_param);

  protected:
  AbstractThread(const char* name, int priority, int stack_size);

  public:
  virtual void main(Param* param) = 0;
  void start(Param* param);
  virtual ~AbstractThread();
};

template <typename Derived, typename Param>
AbstractThread<Derived, Param>::AbstractThread(const char* name, int priority, int stack_size)
    : _AbstractThread(name, priority, stack_size) {}

template <typename Derived, typename Param>
void AbstractThread<Derived, Param>::_main(ThreadMainParam<Derived, Param>* main_param) {
  auto self = main_param->self;
  auto param = main_param->param;
  self->main(param);
  delete main_param->self;
  free(main_param->param);
  free(main_param);
  while (true) {
    vTaskDelay(1000);
  };
  vTaskDelete(nullptr);
}

template <typename Derived, typename Param>
void AbstractThread<Derived, Param>::start(Param* param) {
  auto main_param = static_cast<ThreadMainParam<Derived, Param>*>(malloc(sizeof(ThreadMainParam<Derived, Param>)));
  main_param->param = static_cast<Param*>(malloc(sizeof(Param)));
  memcpy(main_param->param, param, sizeof(Param));
  main_param->self = new Derived(*static_cast<Derived*>(this));
  xTaskCreate(reinterpret_cast<void (*)(void*)>(_main),
              this->name,
              this->stack_size,
              main_param,
              this->priority,
              &this->handle);
}

template <typename Derived, typename Param>
AbstractThread<Derived, Param>::~AbstractThread() {}
