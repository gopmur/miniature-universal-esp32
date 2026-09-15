#pragma once

#include <string>
#include "custom_drivers/can_device_reader.hpp"
#include "custom_drivers/motor/packet.hpp"
#include "esp_twai_types.h"
#include "hal/twai_types.h"

struct MotorFeedback {
  float torque = 0;
  float position = 0;
  float velocity = 0;
  float temperature = 0;
};

enum class MotorDirection {
  FORWARD,
  BACKWARD,
};

class AbstractMotorDriver : public AbstractCanDeviceReader {
  protected:
  int id;
  twai_node_handle_t twai;
  MotorFeedback feedback;
  float max_torque = 0;
  std::string tag = "motor";
  MotorDirection direction;
  float torque_constant = 0;

  twai_frame_header_t make_header();
  virtual MotorPacket make_torque_packet(float torque) = 0;
  virtual MotorPacket make_read_encoder_packet() = 0;
  virtual MotorPacket make_enable_packet() = 0;
  virtual MotorPacket make_disable_packet() = 0;
  virtual MotorPacket make_zero_pos_packet() = 0;
  void send_packet(MotorPacket packet);

  public:
  AbstractMotorDriver(int id,
                      twai_node_handle_t twai,
                      float max_torque,
                      MotorDirection direction,
                      float torque_constant);
  virtual void set_torque(float torque);
  virtual void poll_encoder();
  virtual void enable();
  virtual void disable();
  virtual void zero_pos();
  virtual float get_torque();
  virtual float get_position();
  virtual float get_velocity();
  virtual float get_temperature();
  virtual void init() = 0;
};
