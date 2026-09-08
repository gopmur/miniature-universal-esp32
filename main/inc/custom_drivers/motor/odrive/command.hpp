#pragma once

#include <cstdint>

enum class ODriveMotorCommand : uint8_t {
  SET_AXIS_STATE = 0x07,
  GET_ENCODER_ESTIMATES = 0x09,
  SET_CONTROLLER_MODES = 0x0b,
  SET_INPUT_POS = 0x0c,
  SET_INPUT_VEL = 0x0d,
  SET_INPUT_TORQUE = 0x0e,
  CLEAR_ERRORS = 0x18,
  GET_TORQUES = 0x1c,
  HEARTBEAT = 0x01,
  ABSOULTE_POSITION = 0x19,
};

enum class ODriveMotorAxisState : uint8_t {
  IDLE = 0x01,
  CLOSED_LOOP_CONTROL = 0x08,
};
