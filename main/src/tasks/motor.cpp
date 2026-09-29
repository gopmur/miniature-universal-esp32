#include "tasks/motor.hpp"
#include "boost/preprocessor/comparison/less.hpp"
#include "boost/preprocessor/control/if.hpp"
#include "boost/preprocessor/repetition/repeat.hpp"
#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive.hpp"
#include "helper.hpp"
#include "jaythread/sync.hpp"
#include "sdkconfig.h"
#include "system_logger.hpp"
#include "tasks/can_recv.hpp"

extern twai_node_handle_t twai;
extern CanRecvTask* can_recv_task;

#define MOTOR_ODRIVE(n)                                                                            \
  MotorDirection direction = IS_ENABLED(BOOST_PP_CAT(CONFIG_HEXA_MOTOR_, n##_BACKWARDS))           \
                                 ? MotorDirection::BACKWARD                                        \
                                 : MotorDirection::FORWARD;                                        \
  motors[n] =                                                                                      \
      new ODriveMotorDriver(BOOST_PP_CAT(CONFIG_HEXA_MOTOR_, n##_ID), twai, 0.2, direction, 0.02); \
  can_recv_task->bind(motors[n]->get_id() << 5, ~((1 << 5) - 1), motors[n]);

#define NEW_MOTOR(z, n, data)                                          \
  if (IS_ENABLED(BOOST_PP_CAT(CONFIG_HEXA_MOTOR_, n##_TYPE_ODRIVE))) { \
    MOTOR_ODRIVE(n)                                                    \
  }

MotorTask::MotorTask() {
  BOOST_PP_REPEAT(2, NEW_MOTOR, ~);
  torques.fill(0);
}

void MotorTask::set_torque(size_t motor_index, float torque) {
  if (motor_index >= motors.size()) {
    LOGW("cannot set torque for motor %d. index out of bounds", motor_index);
    return;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("cannot set torque for motor %d. doesn't exist", motor_index);
    return;
  }
  motor->set_torque(torque);
}

void MotorTask::enable_all() {
  for (size_t i = 0; i < motors.size(); i++) {
    enable(i);
  }
}

void MotorTask::disable_all() {
  for (size_t i = 0; i < motors.size(); i++) {
    disable(i);
  }
}

void MotorTask::enable(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("cannot enable motor %d. index out of bounds", motor_index);
    return;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("cannot enable motor %d. doesn't exist", motor_index);
    return;
  }
  motor->set_torque(0);
  motor->enable();
  motor->set_torque(0);
}

void MotorTask::disable(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("cannot disable motor %d. index out of bounds", motor_index);
    return;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("cannot disable motor %d. doesn't exist", motor_index);
    return;
  }
  motor->set_torque(0);
  motor->enable();
  motor->set_torque(0);
}

void MotorTask::init(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("cannot initialize motor %d. index out of bounds", motor_index);
    return;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("cannot initialize motor %d. doesn't exist", motor_index);
    return;
  }
  motor->init();
}

void MotorTask::init_all() {
  for (int i = 0; i < motors.size(); i++) {
    init(i);
  }
}

float MotorTask::get_velocity(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("failed to get velocity of motor %d. index out of bounds", motor_index);
    return 0;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("failed to get velocity of motor %d. doesn't exist", motor_index);
    return 0;
  };
  return motor->get_velocity();
}

float MotorTask::get_position(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("failed to get position of motor %d. index out of bounds", motor_index);
    return 0;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("failed to get position of motor %d. doesn't exist", motor_index);
    return 0;
  };
  return motor->get_position();
}

void MotorTask::zero_pos_all() {
  for (int i = 0; i < motors.size(); i++) {
    zero_pos(i);
  }
}

void MotorTask::zero_pos(size_t motor_index) {
  if (motor_index >= motors.size()) {
    LOGW("failed to set motor %d to zero pos. index out of bounds", motor_index);
    return;
  }
  auto motor = motors[motor_index];
  if (motor == nullptr) {
    LOGW("failed to set motor %d to zero pos. doesn't exist", motor_index);
    return;
  }
  motor->zero_pos();
}

void MotorTask::main() {
  init_all();
  disable_all();
  while (true) {
    for (int i = 0; i < motors.size(); i++) {
      auto motor = motors[i];
      auto torque = torques[i];
      auto status = motor->get_status();
      if (status == MotorStatus::OK) {
        motor->set_torque(torque);
      } else {
        motor->set_torque(0);
      }
    }
    Sync::sleep(10);
  }
}