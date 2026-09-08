// #pragma once

// #include "custom_drivers/motor.hpp"

// class SmcMotorDriver : public AbstractMotorDriver {
//   private:
//   twai_frame_t make_torque_packet(float torque);
//   twai_frame_t make_read_encoder_packet();
//   twai_frame_t make_enable_packet();
//   twai_frame_t make_disable_packet();
//   twai_frame_t make_zero_pos_packet();
//   static int torque_float_to_uint(float x, int bits);

//   public:
//   SmcMotorDriver(int id, twai_node_handle_t twai);
// };
