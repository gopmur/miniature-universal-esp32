#pragma once

#include "custom_drivers/imu.hpp"
#include "custom_drivers/imu/i2c_imu.hpp"
#include "hal/i2c_types.h"
#include "icm20948.h"
#include "system_logger.hpp"

class I2cIcm20948 : public I2cImu {
  MAKE_LOGGABLE("i2c_icm20948");

  private:
  icm20948_device_t icm;

  public:
  I2cIcm20948(i2c_port_t i2c_port, bool ad0);
  void init();
  bool poll();
};