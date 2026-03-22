#pragma once

#include <algorithm>
#include <cstring>
#include <vector>
#include "freertos/idf_additions.h"
#include "ipc/mutex.hpp"

class Thread {
  private:
  uint32_t last_total_runtime = 0;
  uint32_t last_service_runtime = 0;
  float cpu_usage = 0;
  uint32_t min_free_stack;
  Thread() = default;
  float calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime);
  float calculate_max_stack_usage(uint32_t stack_high_water_mark);

  protected:
  static Mutex thread_list_mutex;
  char* name;
  int priority;
  TaskHandle_t handle;
  void wait_for_notification();
  void wait_for_notification(int ticks_to_wait);
  Thread(const char* name, int priority, int stack_size);
  static std::vector<Thread*> thread_list;

  public:
  const int stack_size = 0;
  Thread(Thread& other);

  void suspend();
  void resume();
  void resume_from_isr();
  void notify();
  bool notify_from_isr();
  void update_runtime_stats();
  const char* get_name();
  float get_cpu_usage();
  uint32_t get_min_free_stack();
  TaskHandle_t get_handle();
  ~Thread();
  static const std::vector<Thread*> get_thread_list();
};

class ThreadWrapper : public Thread {
  public:
  ThreadWrapper(TaskHandle_t handle);
  void register_to_list();
};

template <typename Derived, typename Param>
class ThreadWithArg;

template <typename Derived, typename Param>
struct ThreadMainParam {
  ThreadWithArg<Derived, Param>* self;
  Param* param;
};

template <typename Derived, typename Param>
class ThreadWithArg : public Thread {
  private:
  static void _main(ThreadMainParam<Derived, Param>* main_param);

  protected:
  ThreadWithArg(const char* name, int priority, int stack_size);

  public:
  virtual void main(Param* param) = 0;
  void start(Param* param);
  virtual ~ThreadWithArg() = default;
};

template <typename Derived, typename Param>
ThreadWithArg<Derived, Param>::ThreadWithArg(const char* name, int priority, int stack_size)
    : Thread(name, priority, stack_size) {}

template <typename Derived, typename Param>
void ThreadWithArg<Derived, Param>::_main(ThreadMainParam<Derived, Param>* main_param) {
  auto self = main_param->self;
  auto param = main_param->param;
  self->main(param);
  Thread::thread_list_mutex.take();
  auto index_in_thread_list =
      std::find(Thread::thread_list.begin(), Thread::thread_list.end(), main_param->self);
  Thread::thread_list.erase(index_in_thread_list);
  Thread::thread_list_mutex.give();
  delete main_param->self;
  free(main_param->param);
  free(main_param);
  vTaskDelete(nullptr);
}

template <typename Derived, typename Param>
void ThreadWithArg<Derived, Param>::start(Param* param) {
  auto main_param = static_cast<ThreadMainParam<Derived, Param>*>(
      malloc(sizeof(ThreadMainParam<Derived, Param>)));
  main_param->param = static_cast<Param*>(malloc(sizeof(Param)));
  memcpy(main_param->param, param, sizeof(Param));
  main_param->self = new Derived(*static_cast<Derived*>(this));
  xTaskCreate(reinterpret_cast<void (*)(void*)>(_main),
              this->name,
              this->stack_size,
              main_param,
              this->priority,
              &this->handle);
  Thread::thread_list_mutex.take();
  Thread::thread_list.push_back(new Thread(*this));
  Thread::thread_list_mutex.give();
}