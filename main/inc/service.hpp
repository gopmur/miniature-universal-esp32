#pragma once

#include "freertos/idf_additions.h"

#define START_SERVICE(NAME, PRIORITY)                                          \
  priority = PRIORITY;                                                         \
  this->thread_id = xTaskCreateStatic(reinterpret_cast<void (*)(void*)>(main), \
                                      NAME,                                    \
                                      stack_size,                              \
                                      this,                                    \
                                      priority,                                \
                                      stack,                                   \
                                      &tcb);

template <int STACK_SIZE>
class Service {
  protected:
  StackType_t stack[STACK_SIZE];
  StaticTask_t tcb;
  int priority;
  TaskHandle_t thread_id;
  void wait_for_notification();

  public:
  static constexpr int stack_size = STACK_SIZE;

  TaskHandle_t get_thread_id();
  void suspend();
  void resume();
  void resume_from_isr();
  void notify();
  // return true if higher a priority
  // task has woken.
  bool notify_from_isr();
  virtual void start() = 0;
};

template <int STACK_SIZE>
TaskHandle_t Service<STACK_SIZE>::get_thread_id() {
  return this->thread_id;
}

template <int STACK_SIZE>
void Service<STACK_SIZE>::suspend() {
  vTaskSuspend(this->thread_id);
}

template <int STACK_SIZE>
void Service<STACK_SIZE>::resume() {
  vTaskResume(this->thread_id);
}

template <int STACK_SIZE>
void Service<STACK_SIZE>::resume_from_isr() {
  xTaskResumeFromISR(this->thread_id);
}

template <int STACK_SIZE>
void Service<STACK_SIZE>::wait_for_notification() {
  ulTaskNotifyTake(true, portMAX_DELAY);
}

template <int STACK_SIZE>
void Service<STACK_SIZE>::notify() {
  xTaskNotifyGive(this->thread_id);
}

template <int STACK_SIZE>
bool Service<STACK_SIZE>::notify_from_isr() {
  BaseType_t higher_priority_task_woken = false;
  vTaskNotifyGiveFromISR(this->thread_id, &higher_priority_task_woken);
  return higher_priority_task_woken;
}
