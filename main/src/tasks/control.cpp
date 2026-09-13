#include "tasks/control.hpp"
#include "jaythread/sync.hpp"
#include "tasks/motor.hpp"

extern MotorTask* motor_task;

void ControlTask::main() {
  while (true) {
    motor_task->set_torque(control_params.manual.left.torque, control_params.manual.right.torque);
    Sync::sleep(10);
  }
}