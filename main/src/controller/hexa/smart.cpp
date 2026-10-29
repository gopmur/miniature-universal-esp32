#include "controller/hexa/smart.hpp"
#include "custom_drivers/nvs.hpp"
#include "nvs.h"

float HexaSmartControlConfigs::get_velocity_threshold() {
  return velocity_threshold;
}

uint32_t HexaSmartControlConfigs::get_right_timeout() {
  return right_timeout;
}
uint32_t HexaSmartControlConfigs::get_left_timeout() {
  return left_timeout;
}
uint32_t HexaSmartControlConfigs::get_right_delay_timeout() {
  return right_delay_timeout;
}
uint32_t HexaSmartControlConfigs::get_left_delay_timeout() {
  return left_delay_timeout;
}

void HexaSmartControlConfigs::set_velocity_threshold(float velocity_threshold) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(velocity_threshold_key, velocity_threshold);
  this->velocity_threshold = velocity_threshold;
}

void HexaSmartControlConfigs::init() {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.init(velocity_threshold_key, 0.8f);
  nvs.init(right_timeout_key, 300u);
  nvs.init(left_timeout_key, 300u);
  nvs.init(right_delay_timeout_key, 20u);
  nvs.init(left_delay_timeout_key, 20u);
}

void HexaSmartControlConfigs::load() {
  Nvs nvs(nvs_namespace, NVS_READONLY);
  velocity_threshold = nvs.get_float(velocity_threshold_key);
  right_timeout = nvs.get_uint32(right_timeout_key);
  left_timeout = nvs.get_uint32(left_timeout_key);
  right_delay_timeout = nvs.get_uint32(right_delay_timeout_key);
  left_delay_timeout = nvs.get_uint32(left_delay_timeout_key);
}

void HexaSmartControlConfigs::set_right_timeout(uint32_t right_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(right_timeout_key, right_timeout);
  this->right_timeout = right_timeout;
}

void HexaSmartControlConfigs::set_left_timeout(uint32_t left_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(left_timeout_key, left_timeout);
  this->left_timeout = left_timeout;
}

void HexaSmartControlConfigs::set_right_delay_timeout(uint32_t right_delay_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(right_delay_timeout_key, right_delay_timeout);
  this->right_delay_timeout = right_delay_timeout;
}
void HexaSmartControlConfigs::set_left_delay_timeout(uint32_t left_delay_timeout) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(left_delay_timeout_key, left_delay_timeout);
  this->left_delay_timeout = left_delay_timeout;
}

float HexaSmartController::calculate_output(const std::vector<float>& x) {
  float velocity = x[1];
  float last = x[2];

  if (last == 0) {
    if (velocity > configs.get_velocity_threshold()) {
      return 1.0;
    } else {
      return 0.0;
    }
  } else {  // last == 1
    if (velocity > configs.get_velocity_threshold()) {
      return 1.0;
    } else if (velocity >= configs.get_velocity_threshold()) {
      return 1.0;
    } else {
      return 0.0;
    }
  }
}

HexaControllerOutput HexaSmartController::run(HexaControllerInput input) {
  HexaControllerOutput output;
  float r_delta_angle = input.right_motor.position - input.left_motor.position;
  float l_delta_angle = input.left_motor.position - input.right_motor.position;

  if ((float)calculate_output({r_delta_angle, input.right_motor.velocity, previous_right_state}) !=
          previous_right_state &&
      fuzzy_timer_r > 300) {
    fuzzy_timer_r = 0;
    right_state =
        (float)calculate_output({r_delta_angle, input.right_motor.velocity, previous_right_state});
  } else {
    right_state = previous_right_state;
  }
  right_activated = (right_state > 0);

  if ((float)calculate_output({l_delta_angle, input.left_motor.velocity, previous_left_state}) !=
          previous_left_state &&
      fuzzy_timer_l > 300) {
    fuzzy_timer_l = 0;
    left_state =
        (float)calculate_output({l_delta_angle, input.left_motor.velocity, previous_left_state});
  } else {
    left_state = previous_left_state;
  }
  if (right_state == 0 && previous_right_state != 0) {
    left_delay_timer = 0;
  }
  if ((right_activated && params.right.torque != 0) || left_delay_timer < 20) {
    left_state = 0;
    if (left_state != previous_left_state) {
      fuzzy_timer_l = 0;
    }
  }
  if (left_state == 0 && previous_left_state != 0) {
    right_delay_timer = 0;
  }
  left_activated = (left_state > 0);
  if ((left_activated && params.left.torque != 0) || right_delay_timer < 20) {
    right_state = 0;
    if (right_state != previous_right_state) {
      fuzzy_timer_r = 0;
    }
  }
  previous_left_state = left_state;
  previous_right_state = right_state;
  fuzzy_timer_r += input.freq.ms();
  fuzzy_timer_l += input.freq.ms();
  right_delay_timer += input.freq.ms();
  left_delay_timer += input.freq.ms();
  float torque_profile_right_value = torque_profile(fuzzy_timer_r, 50);
  float torque_profile_left_value = torque_profile(fuzzy_timer_l, 50);
  output.right_motor.torque = params.right.torque * torque_profile_right_value * right_state;
  output.left_motor.torque = params.left.torque * torque_profile_left_value * left_state;
  return output;
}

void HexaSmartController::reset() {
  previous_left_state = 0;
  previous_right_state = 0;
  fuzzy_timer_r = 1000;
  fuzzy_timer_l = 1000;
  right_state = 0;
  left_state = 0;
  right_activated = false;
  left_activated = false;
}
