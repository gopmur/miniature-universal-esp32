#include "tasks/logger.hpp"
#include <cstdio>
#include "custom_drivers/motor.hpp"
#include "jaythread/sync.hpp"
#include "tasks/imu.hpp"
#include "tasks/motor.hpp"

extern ImuTask* imu_task;
extern MotorTask* motor_task;

void LoggerTask::start_new_log() {
  is_logging = true;
  notify();
}

void LoggerTask::stop_log() {
  is_logging = false;
}

void LoggerTask::write_header() {
  for (int i = 0; i < motor_task->motor_count; i++) {
    fprintf(log_file, "motor_%d_position,", i);
  }
  for (int i = 0; i < motor_task->motor_count; i++) {
    fprintf(log_file, "motor_%d_velocity,", i);
  }
  fprintf(log_file, "imu_gyro_x,imu_gyro_y,imu_gyro_z\n");
}

void LoggerTask::write_data() {
  for (int i = 0; i < motor_task->motor_count; i++) {
    fprintf(log_file, "%f,", motor_task->get_position(i));
  }
  for (int i = 0; i < motor_task->motor_count; i++) {
    fprintf(log_file, "%f,", motor_task->get_velocity(i));
  }
  fprintf(log_file,
          "%f,%f,%f\n",
          imu_task->data.gyro.x,
          imu_task->data.gyro.y,
          imu_task->data.gyro.z);
}

void LoggerTask::main() {
  Sync::wait_for_notification_and_clear();
  while (true) {
    LOGI("log started");
    log_file = fopen("/sd/my_log.csv", "w");
    write_header();
    while (is_logging) {
      write_header();
      Sync::sleep(10);
    }
    LOGI("log stopped");
    fclose(log_file);
    LOGI("log data written");
    Sync::wait_for_notification_and_clear();
  }
}