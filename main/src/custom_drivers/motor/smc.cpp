// #include "driver/motor/smc.hpp"
// #include "driver/motor/smc/command.hpp"
// #include "shared/utils.hpp"

// SmcMotorDriver::SmcMotorDriver(int id, FDCAN_HandleTypeDef* fdcan)
//     : AbstractMotorDriver(id, fdcan) {}

// MotorPacket SmcMotorDriver::make_torque_packet(float tau_ff_in) {
//   float tau_ff = fminf(fmaxf(-2048.0f, tau_ff_in), 2048.0f);
//   uint16_t t_int = torque_float_to_uint(tau_ff, 16);
//   MotorPacket packet;
//   packet.header = make_header();
//   packet.data.fill(0);
//   packet.data[0] = static_cast<uint8_t>(SmcMotorCommand::TORQUE);
//   packet.data[4] = utils::get_low(t_int);
//   packet.data[5] = utils::get_high(t_int);
//   return packet;
// }

// // MotorPacket SmcMotorDriver::make_position_packet( float
// // tau_ff_in); 
// MotorPacket SmcMotorDriver::make_read_encoder_packet() {
//   MotorPacket packet;
//   packet.header = make_header();
//   packet.data.fill(0);
//   packet.data[0] = static_cast<uint8_t>(SmcMotorCommand::READ_ENCODER);
//   return packet;
// }

// MotorPacket SmcMotorDriver::make_enable_packet() {
//   MotorPacket packet;
//   packet.header = make_header();
//   packet.data.fill(0);
//   packet.data[0] = static_cast<uint8_t>(SmcMotorCommand::ENABLE);
//   return packet;
// }
// MotorPacket SmcMotorDriver::make_disable_packet() {
//   MotorPacket packet;
//   packet.header = make_header();
//   packet.data.fill(0);
//   packet.data[0] = static_cast<uint8_t>(SmcMotorCommand::DISABLE);
//   return packet;
// }
// MotorPacket SmcMotorDriver::make_zero_pos_packet() {
//   MotorPacket packet;
//   packet.header = make_header();
//   packet.data.fill(0);
//   packet.data[0] = static_cast<uint8_t>(SmcMotorCommand::ZERO_POS);
//   return packet;
// }

// int SmcMotorDriver::torque_float_to_uint(float x, int bits) {
//   float val = 0;
//   if (x >= 0) {
//     val = x;
//   } else {
//     val = static_cast<float>((1 << bits) - 1) + x;
//   }
//   return static_cast<int>(val);
// }