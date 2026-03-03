#pragma once

#include "freertos/idf_additions.h"
#include "context/monitoring_data.hpp"

/**
 * @brief Macro to start a FreeRTOS service with a given name and priority.
 *
 * This macro initializes the thread with the provided priority,
 * stack, and task control block, and starts the task using `xTaskCreateStatic`.
 *
 * @param NAME Name of the service/task.
 * @param PRIORITY Priority of the service/task.
 */
#define START_SERVICE(NAME)                                                    \
  this->thread_id = xTaskCreateStatic(reinterpret_cast<void (*)(void*)>(main), \
                                      NAME,                                    \
                                      stack_size,                              \
                                      this,                                    \
                                      priority,                                \
                                      stack,                                   \
                                      &tcb);                                   \

/**
 * @brief Base class template for FreeRTOS services.
 *
 * This class provides common operations such as suspend, resume,
 * notification, and thread management for services implemented as tasks.
 *
 * @tparam STACK_SIZE Size of the task stack.
 */
template <int STACK_SIZE>
class AbstractService {
  private:
  uint32_t last_total_runtime = 0;
  uint32_t last_service_runtime = 0;

  protected:
  StackType_t stack[STACK_SIZE]; /**< Stack memory for the task */
  StaticTask_t tcb;              /**< Task control block */
  int priority;                  /**< Task priority */
  TaskHandle_t thread_id;        /**< Handle of the created task */

  /**
   * @brief Waits for a notification from another task or ISR.
   *
   * This function blocks the task indefinitely until a notification
   * is received. It clears any previous notification value before waiting.
   */
  void wait_for_notification();
  void wait_for_notification(int ticks_to_wait);

  public:
  static constexpr int stack_size = STACK_SIZE; /**< Stack size constant */

  /**
   * @brief Constructs a Service with the specified priority.
   *
   * @param priority Priority of the service task.
   */
  AbstractService(int priority);

  /**
   * @brief Returns the FreeRTOS task handle of this service.
   *
   * @return TaskHandle_t Handle of the task.
   */
  TaskHandle_t get_thread_id();

  /**
   * @brief Suspends the service task.
   *
   * The task will not execute until resumed.
   */
  void suspend();

  /**
   * @brief Resumes the service task if it was suspended.
   */
  void resume();

  /**
   * @brief Resumes the service task from an ISR context.
   */
  void resume_from_isr();

  /**
   * @brief Notifies the service task to unblock it.
   */
  void notify();

  /**
   * @brief Notifies the service task from an ISR context.
   *
   * @return true if a higher priority task was woken by this notification.
   */
  bool notify_from_isr();

  float calculate_cpu_usage(uint32_t service_runtime, uint32_t total_runtime);

  /**
   * @brief Starts the service with a given name.
   *
   * This function must set up the task using `xTaskCreateStatic` or similar.
   *
   * @param service_name Name of the service/task.
   */
  void start(const char* service_name);

  /**
   * @brief Pure virtual function that must be implemented by derived classes.
   *
   * This function is the entry point of the service task.
   */
  virtual void start() = 0;
};

template <int STACK_SIZE>
AbstractService<STACK_SIZE>::AbstractService(int priority)
    : priority(priority) {}

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
float AbstractService<STACK_SIZE>::calculate_cpu_usage(uint32_t service_runtime,
                                                       uint32_t total_runtime) {
  uint32_t d_service_runtime = service_runtime - last_service_runtime;
  uint32_t d_total_runtime = total_runtime - last_total_runtime;
  last_service_runtime = service_runtime;
  last_total_runtime = total_runtime;
  return d_service_runtime * 100.0 / d_total_runtime;
}
