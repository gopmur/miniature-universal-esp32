#include "controller/hexa/automatic.hpp"
#include "custom_drivers/nvs.hpp"
#include "nvs.h"

float HexaAutomaticControlConfigs::get_angle_threshold() {
  return angle_threshold;
}

float HexaAutomaticControlConfigs::get_left_velocity_threshold() {
  return left_velocity_threshold;
}

float HexaAutomaticControlConfigs::get_right_velocity_threshold() {
  return right_velocity_threshold;
}

int32_t HexaAutomaticControlConfigs::get_left_timeout() {
  return left_timeout;
}

int32_t HexaAutomaticControlConfigs::get_right_timeout() {
  return right_timeout;
}

void HexaAutomaticControlConfigs::set_angle_threshold(float position_difference) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(angle_threshold_key, position_difference);
  this->angle_threshold = position_difference;
}

void HexaAutomaticControlConfigs::set_left_velocity_threshold(float left_velocity_threshold) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(left_velocity_threshold_key, left_velocity_threshold);
  this->left_velocity_threshold = left_velocity_threshold;
}

void HexaAutomaticControlConfigs::set_right_velocity_threshold(float right_velocity_threshold) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(right_velocity_threshold_key, right_velocity_threshold);
  this->right_velocity_threshold = right_velocity_threshold;
}

void HexaAutomaticControlConfigs::set_right_timeout(int32_t right_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(right_timeout_key, right_timeout);
  this->right_timeout = right_timeout;
}

void HexaAutomaticControlConfigs::set_left_timeout(int32_t left_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(left_timeout_key, left_timeout);
  this->left_timeout = left_timeout;
}

void HexaAutomaticControlConfigs::init() {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.init(left_velocity_threshold_key, 1.431169987);
  nvs.init(right_velocity_threshold_key, 1.431169987);
  nvs.init(angle_threshold_key, 1.431169987);
  nvs.init(left_timeout_key, 200);
  nvs.init(right_timeout_key, 200);
}

void HexaAutomaticControlConfigs::load() {
  Nvs nvs(nvs_namespace, NVS_READONLY);
  left_velocity_threshold = nvs.get_float(left_velocity_threshold_key);
  right_velocity_threshold = nvs.get_float(right_velocity_threshold_key);
  angle_threshold = nvs.get_float(angle_threshold_key);
  right_timeout = nvs.get_float(right_timeout_key);
  left_timeout = nvs.get_float(left_timeout_key);
}

HexaControllerOutput HexaAutomaticController::run(HexaControllerInput input) {
  HexaControllerOutput output;
  float torque_profile_right_value = 0;
  float torque_profile_left_value = 0;
  if (right_timer > configs.get_right_timeout()) {
    ro = false;
  } else if (input.right_motor.velocity > configs.get_right_velocity_threshold() &&
             (input.left_motor.position - input.right_motor.position) >
                 configs.get_angle_threshold() &&
             input.left_motor.velocity <= 0 && ri == false) {
    ro = true;
    ri = true;
    li = false;
  }

  if (left_timer > configs.get_left_timeout()) {
    lo = false;
  } else if (input.left_motor.velocity > configs.get_left_velocity_threshold() &&
             (input.right_motor.position - input.left_motor.position) >
                 configs.get_angle_threshold() &&
             input.right_motor.velocity < 0.0 && li == false) {
    lo = true;
    ri = false;
    li = true;
  }

  if (ro == true) {
    right_timer = right_timer + 1;
    torque_profile_right_value = torque_profile(right_timer, configs.get_right_timeout());
  } else if (ro == false) {
    right_timer = 0;
    torque_profile_right_value = 0;
  }

  if (lo == true) {
    left_timer = left_timer + 1;
    torque_profile_left_value = torque_profile(left_timer, configs.get_left_timeout());
  } else if (lo == false) {
    left_timer = 0;
    torque_profile_left_value = 0;
  }

  output.left_motor.torque = params.left.torque * torque_profile_left_value;
  output.right_motor.torque = params.right.torque * torque_profile_right_value;

  return output;
}

void HexaAutomaticController::reset() {
  right_timer = 0;
  left_timer = 0;
  ro = false;
  ri = false;
  lo = false;
  li = false;
}