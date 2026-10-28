#include "tasks/imu.hpp"
#include <cmath>
#include "custom_drivers/imu/i2c_imu/i2c_icm20948.hpp"
#include "helper/ds/period.hpp"
#include "jaythread/sync.hpp"

void ImuTask::main() {
  imu = new I2cIcm20948(I2C_NUM_0, true);
  auto frequency = Period::from_hz(CONFIG_HEXA_TASKS_IMU_FREQ_HZ);
  while (true) {
    if (imu->poll()) {
      Sync::sleep(frequency.ms());
    } else {
      Sync::sleep(frequency.ms() * 2);
    }
  }
}
