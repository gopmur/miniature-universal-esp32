#pragma once

#include "custom_drivers/imu.hpp"
#include "hal/i2c_types.h"

class I2cImu : public Imu {
  protected:
  i2c_port_t i2c_port;
  uint8_t i2c_address;
  I2cImu(i2c_port_t i2c_port, uint8_t i2c_address);
};