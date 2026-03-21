#pragma once

#include <cstdlib>
#include <cstring>
#include "freertos/idf_additions.h"

template <int STACK_SIZE>
class AbstractService {
  private:
  uint32_t last_total_runtime = 0;
  uint32_t last_service_runtime = 0;
  static void _main(AbstractService* self);

  protected:
  StackType_t stack[STACK_SIZE]; /**< Stack memory for the task */
  StaticTask_t tcb;              /**< Task control block */
  int priority;                  /**< Task priority */
  TaskHandle_t thread_id;        /**< Handle of the created task */
  char* name;

  void wait_for_notification();
  void wait_for_notification(int ticks_to_wait);
  virtual void main() = 0;
  AbstractService(int priority, const char* name);

  public:
  static constexpr int stack_size = STACK_SIZE; /**< Stack size constant */

  TaskHandle_t get_thread_id();

  void suspend();
  void resume();
  void resume_from_isr();
  void notify();
  bool notify_from_isr();
  float calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime);
  void start();
};

template <int STACK_SIZE>
AbstractService<STACK_SIZE>::AbstractService(int priority, const char* name) : priority(priority) {
  this->name = static_cast<char*>(malloc(strlen(name)));
  strcpy(this->name, name);
}

template <int STACK_SIZE>
TaskHandle_t AbstractService<STACK_SIZE>::get_thread_id() {
  return this->thread_id;
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::suspend() {
  vTaskSuspend(this->thread_id);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::resume() {
  vTaskResume(this->thread_id);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::resume_from_isr() {
  xTaskResumeFromISR(this->thread_id);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::wait_for_notification() {
  ulTaskNotifyTake(true, portMAX_DELAY);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::wait_for_notification(int ticks_to_wait) {
  ulTaskNotifyTake(true, ticks_to_wait);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::notify() {
  xTaskNotifyGive(this->thread_id);
}

template <int STACK_SIZE>
bool AbstractService<STACK_SIZE>::notify_from_isr() {
  BaseType_t higher_priority_task_woken = false;
  vTaskNotifyGiveFromISR(this->thread_id, &higher_priority_task_woken);
  return higher_priority_task_woken;
}

template <int STACK_SIZE>
float AbstractService<STACK_SIZE>::calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime) {
  uint32_t d_service_runtime = service_runtime - last_service_runtime;
  uint32_t d_total_runtime = total_runtime - last_total_runtime;
  last_service_runtime = service_runtime;
  last_total_runtime = total_runtime;
  return d_service_runtime * 100.0 / d_total_runtime;
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::start() {
  this->thread_id = xTaskCreateStatic(reinterpret_cast<void (*)(void*)>(AbstractService<STACK_SIZE>::_main),
                                      this->name,
                                      stack_size,
                                      this,
                                      priority,
                                      stack,
                                      &tcb);
}

template <int STACK_SIZE>
void AbstractService<STACK_SIZE>::_main(AbstractService* self) {
  self->main();
  self->suspend();
}