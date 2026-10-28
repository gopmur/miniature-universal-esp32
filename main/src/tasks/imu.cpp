#include "tasks/imu.hpp"
#include <cmath>
#include "custom_drivers/imu/i2c_imu/i2c_icm20948.hpp"  // IWYU pragma: keep
#include "custom_drivers/imu/mock.hpp"                  // IWYU pragma: keep
#include "helper/ds/period.hpp"
#include "jaythread/sync.hpp"

void ImuTask::main() {
#ifdef CONFIG_HEXA_IMU_TYPE_I2C_ICM20948
  #ifdef CONFIG_HEXA_IMU_I2C_ICM20948_AD1
  bool ad1 = true;
  #else
  bool ad1 = false;
  #endif
  imu = new I2cIcm20948(I2C_NUM_0, ad1);
#elifdef CONFIG_HEXA_IMU_TYPE_MOCK
  imu = new MockImu();
#endif
  auto frequency = Period::from_hz(CONFIG_HEXA_TASKS_IMU_FREQ_HZ);
  while (true) {
    data = imu->get_data();
    if (imu->poll()) {
      Sync::sleep(frequency.ms());
    } else {
      Sync::sleep(frequency.ms() * 2);
    }
  }
}
