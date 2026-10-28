#include "controller/hexa/semi_automatic.hpp"
#include <cstdlib>
#include "controller.hpp"
#include "custom_drivers/nvs.hpp"
#include "nvs.h"

float HexaSemiautomaticControlConfigs::get_start_assist_angle() {
  return start_assist_angle;
}

float HexaSemiautomaticControlConfigs::get_stop_assist_angle() {
  return stop_assist_angle;
}

float HexaSemiautomaticControlConfigs::get_epsilon() {
  return epsilon;
}

void HexaSemiautomaticControlConfigs::set_start_assist_angle(float start_assist_angle) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(start_assist_angle_key, start_assist_angle);
  this->start_assist_angle = start_assist_angle;
}

void HexaSemiautomaticControlConfigs::set_stop_assist_angle(float stop_assist_angle) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(stop_assist_angle_key, stop_assist_angle);
  this->stop_assist_angle = stop_assist_angle;
}

void HexaSemiautomaticControlConfigs::set_epsilon(float epsilon) {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.set(epsilon_key, epsilon);
  this->epsilon = epsilon;
}

void HexaSemiautomaticControlConfigs::load() {
  Nvs nvs(nvs_namespace, NVS_READONLY);
  start_assist_angle = nvs.get_float(start_assist_angle_key);
  stop_assist_angle = nvs.get_float(stop_assist_angle_key);
  epsilon = nvs.get_float(epsilon_key);
}

void HexaSemiautomaticControlConfigs::init() {
  Nvs nvs(nvs_namespace, NVS_READWRITE);
  nvs.init(start_assist_angle_key, 0.287979327f);
  nvs.init(stop_assist_angle_key, 2.583087293f);
  nvs.init(epsilon_key, 0.02864837f);
}

HexaControllerOutput HexaSemiautomaticController::run(HexaControllerInput input) {
  HexaControllerOutput output;
  output.left_motor.torque = 0;
  output.right_motor.torque = 0;
  if (params.weak_leg == Leg::RIGHT) {
    if ((input.left_motor.position - input.right_motor.position) >
            configs.get_start_assist_angle() &&
        std::abs(input.left_motor.velocity) < configs.get_epsilon() &&
        std::abs(input.right_motor.velocity) < configs.get_epsilon()) {
      activate_assistance = true;
    }
    if (activate_assistance) {
      d_t += 10;
      if (d_t >= params.right.delay) {
        r_t += 10;
        output.right_motor.torque = params.right.torque;
      }
      if ((input.left_motor.position - input.right_motor.position) <
              -configs.get_stop_assist_angle() ||
          r_t > params.right.timeout) {
        activate_assistance = false;
        r_t = 0;
        d_t = 0;
        output.right_motor.torque = 0;
      }
    }
    output.left_motor.torque = 0;
  } else {
    if ((input.right_motor.position - input.left_motor.position) >
            configs.get_start_assist_angle() &&
        std::abs(input.left_motor.velocity) < configs.get_epsilon() &&
        std::abs(input.right_motor.velocity) < configs.get_epsilon()) {
      activate_assistance = true;
    }
    if (activate_assistance) {
      d_t += 10;
      if (d_t >= params.left.delay) {
        l_t += 10;
        output.left_motor.torque = params.left.torque;
      }
      if ((input.right_motor.position - input.left_motor.position) <
              -configs.get_stop_assist_angle() ||
          l_t > params.left.timeout) {
        activate_assistance = false;
        l_t = 0;
        d_t = 0;
        output.left_motor.torque = 0;
      }
    }
    output.right_motor.torque = 0;
  }
  return output;
}

void HexaSemiautomaticController::reset() {
  d_t = 0;
  r_t = 0;
  l_t = 0;
  activate_assistance = false;
}
