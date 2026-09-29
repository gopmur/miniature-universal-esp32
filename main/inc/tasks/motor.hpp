#pragma once

#include <array>
#include "custom_drivers/motor.hpp"
#include "jaythread/thread.hpp"
#include "sdkconfig.h"

class MotorTask : public Thread {
  MAKE_LOGGABLE("motor_task");

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

  private:
  std::array<float, CONFIG_HEXA_MOTOR_COUNT> torques;
  std::array<AbstractMotorDriver*, CONFIG_HEXA_MOTOR_COUNT> motors;
  void main();
};