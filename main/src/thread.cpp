#include "thread.hpp"
#include "freertos/idf_additions.h"

#include "esp_log.h"

std::vector<Thread*> Thread::thread_list;
Mutex Thread::thread_list_mutex;

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
};

float Thread::calculate_max_stack_usage(uint32_t stack_high_water_mark) {
  return this->stack_size - stack_high_water_mark;
}

void Thread::update_runtime_stats() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-cxx-extension"
  TaskStatus_t runtime_status[32];
#pragma clang diagnostic pop
  int task_count = uxTaskGetNumberOfTasks();
  uint32_t total_runtime;
  uint32_t service_runtime;
  uint32_t stack_high_water_mark;
  uxTaskGetSystemState(runtime_status, 32, &total_runtime);
  for (int i = 0; i < task_count; i++) {
    auto task_status = runtime_status[i];
    if (this->handle == task_status.xHandle) {
      service_runtime = task_status.ulRunTimeCounter;
      stack_high_water_mark = task_status.usStackHighWaterMark;
    }
  }
  this->cpu_usage = this->calculate_cpu_usage(service_runtime, total_runtime);
  this->min_free_stack = stack_high_water_mark;
}

float Thread::get_cpu_usage() {
  return this->cpu_usage;
}

uint32_t Thread::get_min_free_stack() {
  return this->min_free_stack;
}

TaskHandle_t Thread::get_handle() {
  return this->handle;
}

Thread::Thread(Thread& other)
    : priority(other.priority), handle(other.handle), stack_size(other.stack_size) {
  this->name = static_cast<char*>(malloc(strlen(other.name)));
  strcpy(this->name, other.name);
}

const char* Thread::get_name() {
  return this->name;
}

const std::vector<Thread*> Thread::get_thread_list() {
  thread_list_mutex.take();
  auto thread_list_copy = Thread::thread_list;
  thread_list_mutex.give();
  return thread_list_copy;
}

ThreadWrapper::ThreadWrapper(TaskHandle_t handle)
    : Thread(pcTaskGetName(handle), uxTaskPriorityGet(handle), 0  ) {
  this->handle = handle;
}

void ThreadWrapper::register_to_list() {
  thread_list_mutex.take();
  bool already_registered = false;
  for (const auto thread : thread_list) {
    if (thread->get_handle() == this->handle) {
      already_registered = true;
      break;
    }
  }
  if (!already_registered) {
    thread_list.push_back(new Thread(*this));
  }
  thread_list_mutex.give();
}