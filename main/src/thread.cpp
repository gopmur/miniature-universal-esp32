#include "thread.hpp"

_AbstractThread::_AbstractThread(const char* name, int priority, int stack_size)
    : priority(priority), stack_size(stack_size) {
  this->name = static_cast<char*>(malloc(strlen(name) + 1));
  strcpy(this->name, name);
}

_AbstractThread::~_AbstractThread() {
  free(this->name);
}
void _AbstractThread::suspend() {
  vTaskSuspend(this->handle);
}

void _AbstractThread::resume() {
  vTaskResume(this->handle);
}

void _AbstractThread::resume_from_isr() {
  xTaskResumeFromISR(this->handle);
}

void _AbstractThread::wait_for_notification() {
  ulTaskNotifyTake(true, portMAX_DELAY);
}

void _AbstractThread::wait_for_notification(int ticks_to_wait) {
  ulTaskNotifyTake(true, ticks_to_wait);
}

void _AbstractThread::notify() {
  xTaskNotifyGive(this->handle);
}

bool _AbstractThread::notify_from_isr() {
  BaseType_t higher_priority_task_woken = false;
  vTaskNotifyGiveFromISR(this->handle, &higher_priority_task_woken);
  return higher_priority_task_woken;
}

float _AbstractThread::calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime) {
  uint32_t d_service_runtime = service_runtime - last_service_runtime;
  uint32_t d_total_runtime = total_runtime - last_total_runtime;
  last_service_runtime = service_runtime;
  last_total_runtime = total_runtime;
  return d_service_runtime * 100.0 / d_total_runtime;
}

TaskHandle_t _AbstractThread::get_handle() {
  return this->handle;
}

_AbstractThread::_AbstractThread(_AbstractThread& other)
    : priority(other.priority), handle(other.handle), stack_size(other.stack_size) {
  this->name = static_cast<char*>(malloc(strlen(other.name)));
  strcpy(this->name, other.name);
}