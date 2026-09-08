#include "tasks/imu.hpp"
#include <cmath>
#include "assert.h"
#include "esp_log.h"
#include "icm20948.h"
#include "icm20948_i2c.h"
#include "jaythread/sync.hpp"

void ImuTask::main() {
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

  bool success = true;
  success &= (icm20948_init_dmp_sensor_with_defaults(&icm) == ICM_20948_STAT_OK);

  success &= (inv_icm20948_enable_dmp_sensor(&icm, INV_ICM20948_SENSOR_ORIENTATION, 1) ==
              ICM_20948_STAT_OK);

  success &= (inv_icm20948_set_dmp_sensor_period(&icm, DMP_ODR_Reg_Quat9, 0) == ICM_20948_STAT_OK);
  success &= (icm20948_enable_fifo(&icm, true) == ICM_20948_STAT_OK);
  success &= (icm20948_enable_dmp(&icm, 1) == ICM_20948_STAT_OK);
  success &= (icm20948_reset_dmp(&icm) == ICM_20948_STAT_OK);
  success &= (icm20948_reset_fifo(&icm) == ICM_20948_STAT_OK);

  if (success) {
    ESP_LOGI(__FILENAME__, "DMP enabled!");
  } else {
    ESP_LOGE(__FILENAME__, "Enable DMP failed!");
    while (1)
      ;
  }
  while (true) {
    icm_20948_DMP_data_t icm_data;
    icm20948_status_e status = inv_icm20948_read_dmp_data(&icm, &icm_data);
    if ((status == ICM_20948_STAT_OK) || (status == ICM_20948_STAT_FIFO_MORE_DATA_AVAIL)) {
      if ((icm_data.header & DMP_header_bitmap_Quat9) > 0) {
        double q1 = ((double)icm_data.Quat9.Data.Q1) / 1073741824.0;
        double q2 = ((double)icm_data.Quat9.Data.Q2) / 1073741824.0;
        double q3 = ((double)icm_data.Quat9.Data.Q3) / 1073741824.0;
        double q0 = std::sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
        double q_sum = (q1 * q1) + (q2 * q2) + (q3 * q3);
        if (q_sum >= 1.0) {
          q0 = 0.0;
        } else {
          q0 = sqrt(1.0 - q_sum);
        }
        float q2sqr = q2 * q2;
        // Roll
        float t0 = +2.0 * (q0 * q1 + q2 * q3);
        float t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
        float roll = atan2(t0, t1) * 180.0 / M_PI;

        // Pitch
        float t2 = +2.0 * (q0 * q2 - q3 * q1);
        t2 = (t2 > 1.0) ? 1.0 : t2;  // Existing clamp is excellent
        t2 = (t2 < -1.0) ? -1.0 : t2;
        float pitch = asin(t2) * 180.0 / M_PI;

        // Yaw
        float t3 = +2.0 * (q0 * q3 + q1 * q2);
        float t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
        float yaw = atan2(t3, t4) * 180.0 / M_PI;
        
        data.gyro.x = roll;
        data.gyro.y = pitch;
        data.gyro.z = yaw;
      }

      if (status != ICM_20948_STAT_FIFO_MORE_DATA_AVAIL) {
        Sync::sleep(10);
      }
      else {
        Sync::sleep(5);
      }
    }
  }
};
