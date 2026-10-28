#include "custom_drivers/imu/i2c_imu.hpp"

I2cImu::I2cImu(i2c_port_t i2c_port, uint8_t i2c_address)
    : i2c_port(i2c_port), i2c_address(i2c_address) {}