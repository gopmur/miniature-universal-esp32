// #include "custom_drivers/motor/t_motor.hpp"
// #include "custom_drivers/motor/t_motor/command.hpp"
// #include "esp_twai_types.h"

// TMotorDriver::TMotorDriver(int id, twai_node_handle_t fdcan) : AbstractMotorDriver(id, fdcan) {}

// twai_frame_t TMotorDriver::make_torque_packet(float torque) {
//   twai_frame_t packet;
//   packet.header = make_header();

//   float tau_ff = fminf(fmaxf(-16.0f, torque), 16.0f);

//   int p_int = 0;
//   int v_int = 0;
//   int kp_int = 0;
//   int kd_int = 0;
//   int t_int = float_to_uint(tau_ff, -65.0f, 65.0f, 12);

//   packet.data.fill(0);
//   packet.data[0] = static_cast<unsigned char>(p_int >> 8);
//   packet.data[1] = static_cast<unsigned char>(p_int & 0xFF);
//   packet.data[2] = static_cast<unsigned char>(v_int >> 4);
//   packet.data[3] = static_cast<unsigned char>(((v_int & 0xF) << 4) | (kp_int >> 8));
//   packet.data[4] = static_cast<unsigned char>(kp_int & 0xFF);
//   packet.data[5] = static_cast<unsigned char>(kd_int >> 4);
//   packet.data[6] = static_cast<unsigned char>(((kd_int & 0xF) << 4) | (t_int >> 8));
//   packet.data[7] = static_cast<unsigned char>(t_int & 0xFF);

//   return packet;
// }

// twai_frame_t TMotorDriver::make_read_encoder_packet() {
//   twai_frame_t packet;
//   packet.header = make_header();
//   return packet;
// };

// twai_frame_t TMotorDriver::make_enable_packet() {
//   twai_frame_t packet;
//   packet.header = make_header();
//   packet.data.fill(0xff);
//   packet.data[7] = static_cast<uint8_t>(TMotorCommand::ENABLE);
//   return packet;
// }
// twai_frame_t TMotorDriver::make_disable_packet() {
//   twai_frame_t packet;
//   packet.header = make_header();
//   packet.data.fill(0xff);
//   packet.data[7] = static_cast<uint8_t>(TMotorCommand::DISABLE);
//   return packet;
// }

// twai_frame_t TMotorDriver::make_zero_pos_packet() {
//   twai_frame_t packet;
//   packet.header = make_header();
//   packet.data.fill(0xff);
//   packet.data[7] = static_cast<uint8_t>(TMotorCommand::ZERO_POS);
//   return packet;
// }

// int TMotorDriver::float_to_uint(float x, float x_min, float x_max, int bits) {
//   float span = x_max - x_min;
//   float offset = x_min;
//   return (int)((x - offset) * ((float)((1 << bits) - 1)) / span);
// }