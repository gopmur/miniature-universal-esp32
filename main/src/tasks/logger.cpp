#include "tasks/logger.hpp"
#include <cstdio>
#include "custom_drivers/motor.hpp"
#include "esp_log.h"
#include "jaythread/sync.hpp"
#include "tasks/imu.hpp"

extern AbstractMotorDriver* left_motor;
extern AbstractMotorDriver* right_motor;
extern ImuTask* imu_task;

void LoggerTask::start_new_log() {
  is_logging = true;
  notify();
}

void LoggerTask::stop_log() {
  is_logging = false;
}

void LoggerTask::main() {
  Sync::wait_for_notification_and_clear();
  while (true) {
    ESP_LOGI(tag.c_str(), "log started");
    auto log_file = fopen("/sd/log.csv", "w");
    fprintf(log_file,
            "left_motor_position,left_motor_velocity,right_motor_position,right_motor_position,"
            "imu_gyro_x,imu_gyro_y,imu_gyro_z");
    while (is_logging) {
      fprintf(log_file,
              "%f,%f,%f,%f,%f,%f,%f",
              left_motor->get_position(),
              left_motor->get_velocity(),
              right_motor->get_position(),
              right_motor->get_velocity(),
              imu_task->data.gyro.x,
              imu_task->data.gyro.y,
              imu_task->data.gyro.z);
      Sync::sleep(10);
    }
    ESP_LOGI(tag.c_str(), "log stopped");
    fclose(log_file);
    ESP_LOGI(tag.c_str(), "log data written");
    Sync::wait_for_notification_and_clear();
  }
}