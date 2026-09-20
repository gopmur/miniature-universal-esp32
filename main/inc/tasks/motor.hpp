#pragma once

#include "custom_drivers/motor.hpp"
#include "jaythread/thread.hpp"

class MotorTask : public Thread {
  MAKE_LOGGABLE("motor_task");

  public:
  MotorTask(AbstractMotorDriver* left_motor, AbstractMotorDriver* right_motor);
  void set_left_torque(float torque);
  void set_right_torque(float torque);
  void set_torque(float left_torque, float right_torque);
  void enable_left();
  void enable_right();
  void disable_left();
  void disable_right();
  void enable();
  void disable();

  private:
  float left_torque = 0;
  float right_torque = 0;
  AbstractMotorDriver* left_motor;
  AbstractMotorDriver* right_motor;
  void main();
};