#include "thread.hpp"

Thread::Thread(const char* name, int priority, int stack_size)
    : priority(priority), stack_size(stack_size) {
  this->name = static_cast<char*>(malloc(strlen(name) + 1));
  strcpy(this->name, name);
}

Thread::~Thread() {
  free(this->name);
}

void Thread::suspend() {
  vTaskSuspend(this->handle);
}

void Thread::resume() {
  vTaskResume(this->handle);
}

void Thread::resume_from_isr() {
  xTaskResumeFromISR(this->handle);
}

void Thread::wait_for_notification() {
  ulTaskNotifyTake(true, portMAX_DELAY);
}

void Thread::wait_for_notification(int ticks_to_wait) {
  ulTaskNotifyTake(true, ticks_to_wait);
}

void Thread::notify() {
  xTaskNotifyGive(this->handle);
}

bool Thread::notify_from_isr() {
  BaseType_t higher_priority_task_woken = false;
  vTaskNotifyGiveFromISR(this->handle, &higher_priority_task_woken);
  return higher_priority_task_woken;
}

float Thread::calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime) {
  uint32_t d_service_runtime = service_runtime - last_service_runtime;
  uint32_t d_total_runtime = total_runtime - last_total_runtime;
  last_service_runtime = service_runtime;
  last_total_runtime = total_runtime;
  return d_service_runtime * 100.0 / d_total_runtime;
}

TaskHandle_t Thread::get_handle() {
  return this->handle;
}

Thread::Thread(Thread& other)
    : priority(other.priority), handle(other.handle), stack_size(other.stack_size) {
  this->name = static_cast<char*>(malloc(strlen(other.name)));
  strcpy(this->name, other.name);
}