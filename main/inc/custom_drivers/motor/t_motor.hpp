// #pragma once

// #include "custom_drivers/motor.hpp"

// class TMotorDriver : public AbstractMotorDriver {
//   private:
//   twai_frame_t make_torque_packet(float tau_ff_in);
//   twai_frame_t make_read_encoder_packet();
//   twai_frame_t make_enable_packet();
//   twai_frame_t make_disable_packet();
//   twai_frame_t make_zero_pos_packet();
//   static int float_to_uint(float x, float x_min, float x_max, int bits);

//   public:
//   TMotorDriver(int id, twai_node_handle_t twai);
// };
