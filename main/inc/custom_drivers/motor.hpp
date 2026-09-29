#pragma once

#include "custom_drivers/can.hpp"
#include "custom_drivers/can_device_reader.hpp"

#include "jaythread/timer.hpp"
#include "system_logger.hpp"

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

class AbstractMotorDriver;

class MotorTimeoutTimer : public Timer {
  MAKE_LOGGABLE("motor_timeout_timer");

  private:
  AbstractMotorDriver* motor;

  public:
  void init(AbstractMotorDriver* motor);
  void main();
};

enum class MotorStatus {
  OK,
  TIMEDOUT,
  UNINITIALIZED,
  CAN_ERROR,
};

class AbstractMotorDriver : public AbstractCanDeviceReader {
  MAKE_LOGGABLE("motor_driver");

  friend class MotorTimeoutTimer;
  private:
  MotorStatus status = MotorStatus::UNINITIALIZED;
  Mutex status_mutex;
  int id;

  protected:
  MotorTimeoutTimer timeout_timer;
  MotorFeedback feedback;
  Can can;
  float max_torque = 0;
  MotorDirection direction;
  float torque_constant = 0;

  twai_frame_header_t make_header();
  virtual CanPacket make_torque_packet(float torque) = 0;
  virtual CanPacket make_read_encoder_packet() = 0;
  virtual CanPacket make_enable_packet() = 0;
  virtual CanPacket make_disable_packet() = 0;
  virtual CanPacket make_zero_pos_packet() = 0;
  void send_packet(CanPacket packet);
  float apply_direction(float value);
  void consume_position(float position);
  void consume_velocity(float velocity);
  void consume_torque(float torque);

  public:
  MotorStatus get_status();
  AbstractMotorDriver(int id,
                      Can can,
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
  virtual void init();
  virtual void consume(CanPacket packet);
  int get_id();
};
