#include "custom_drivers/imu/mock.hpp"
#include <cmath>

void MockImu::init() {
  t = 0;
}

bool MockImu::poll() {
  data.accel.x = sin(t);
  data.accel.y = sin(t + 0.5);
  data.accel.z = sin(2 * t + 0.2);
  data.gyro.x = cos(t) * M_PI;
  data.gyro.y = cos(t + 0.5) * M_PI;
  data.gyro.z = cos(2 * t + 0.2) * M_PI;
  data.temp = sin(t - 0.5);
  t += t_step;
  return true;
}