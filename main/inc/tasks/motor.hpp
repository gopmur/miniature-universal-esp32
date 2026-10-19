#pragma once

#include <array>
#include "custom_drivers/motor.hpp"
#include "custom_drivers/nvs.hpp"
#include "jaythread/thread.hpp"
#include "nvs.h"
#include "sdkconfig.h"

class MotorTask : public Thread {
  MAKE_LOGGABLE("motor_task");

  private:
  static constexpr const char* torque_constant_key = "trq_const";
  static constexpr const char* torque_limit_key = "trq_limit";

  void init_nvs_torque_constant(size_t motor_index);
  void init_nvs_torque_limit(size_t motor_indx);
  void init_nvs();
  std::string get_nvs_namespace(size_t motor_index);
  Nvs get_nvs(size_t motor_index, nvs_open_mode_t nvs_open_mode);
  void load_nvs();
  void load_nvs_torque_constant(size_t motor_index);
  void load_nvs_torque_limit(size_t motor_index);

  public:
  static constexpr size_t motor_count = CONFIG_HEXA_MOTOR_COUNT;
  MotorTask();
  void set_torque(size_t motor_index, float torque);
  void enable(size_t motor_index);
  void disable(size_t motor_index);
  void enable_all();
  void disable_all();
  void init_all();
  void init(size_t motor_index);
  void zero_pos_all();
  void zero_pos(size_t motor_index);
  float get_position(size_t motor_index);
  float get_velocity(size_t motor_index);
  float get_torque(size_t motor_index);
  void set_torque_constant(size_t motor_index, float torque_constant);
  void set_torque_limit(size_t motor_index, float torque_limit);
  float get_torque_constant(size_t motor_index);
  float get_torque_limit(size_t motor_index);
  MotorStatus get_status(size_t motor_index);
  MotorDirection get_direction(size_t motor_index);
  int get_can_id(size_t motor_index);

  private:
  std::array<float, CONFIG_HEXA_MOTOR_COUNT> torques;
  std::array<AbstractMotorDriver*, CONFIG_HEXA_MOTOR_COUNT> motors;
  void main();
};