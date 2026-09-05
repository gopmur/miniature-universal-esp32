#include "services/imu.hpp"
#include "esp_log.h"
#include "icm20948.h"
#include "icm20948_i2c.h"
#include "jaythread/sync.hpp"

void ImuThread::main() {
  icm0948_config_i2c_t icm_config = {
      .i2c_port = I2C_NUM_0,
      .i2c_addr = ICM_20948_I2C_ADDR_AD0,
  };
  icm20948_device_t icm;

  icm20948_init_i2c(&icm, &icm_config);
  while (icm20948_check_id(&icm) != ICM_20948_STAT_OK) {
    ESP_LOGE(tag, "check id failed");
    Sync::sleep(1000);
  }
  ESP_LOGI(tag, "check id passed");
  icm20948_status_e stat = ICM_20948_STAT_ERR;
  uint8_t whoami = 0x00;
  while ((stat != ICM_20948_STAT_OK) || (whoami != ICM_20948_WHOAMI)) {
    whoami = 0x00;
    stat = icm20948_get_who_am_i(&icm, &whoami);
    ESP_LOGE("ICM", "whoami does not match (0x %d). Halting...", whoami);
    Sync::sleep(1000);
  }
  icm20948_sw_reset(&icm);
  Sync::sleep(250);
  icm20948_internal_sensor_id_bm sensors =
      (icm20948_internal_sensor_id_bm)(ICM_20948_INTERNAL_ACC | ICM_20948_INTERNAL_GYR);
  icm20948_set_sample_mode(&icm, sensors, SAMPLE_MODE_CONTINUOUS);

  icm20948_fss_t fss;
  fss.a = GPM_2;
  fss.g = DPS_250;
  icm20948_set_full_scale(&icm, sensors, fss);

  icm20948_dlpcfg_t dlpcfg;
  dlpcfg.a = ACC_D473BW_N499BW;
  dlpcfg.g = GYR_D361BW4_N376BW5;
  icm20948_set_dlpf_cfg(&icm, sensors, dlpcfg);

  icm20948_enable_dlpf(&icm, ICM_20948_INTERNAL_ACC, false);
  icm20948_enable_dlpf(&icm, ICM_20948_INTERNAL_GYR, false);

  icm20948_sleep(&icm, false);
  icm20948_low_power(&icm, false);
  while (true) {
    icm20948_agmt_t agmt;
    if (icm20948_get_agmt(&icm, &agmt) == ICM_20948_STAT_OK) {
      data.accel.x = agmt.acc.axes.x;
      data.accel.y = agmt.acc.axes.y;
      data.accel.z = agmt.acc.axes.z;

      data.gyro.x = agmt.gyr.axes.x;
      data.gyro.y = agmt.gyr.axes.y;
      data.gyro.z = agmt.gyr.axes.z;

      data.temp = agmt.tmp.val;
    } else {
      ESP_LOGE("tag", "data acquisition failed");
    }
    Sync::sleep(100);
  }
};
