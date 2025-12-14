#include <array>
#include <cstdint>

enum class UartPacketType : uint8_t {
  START,
  STOP,
  SET_LEFT_TORQUE,
  SET_RIGHT_TORQUE,
};

union UartPayload {
  int32_t torque;
};

struct UartPacket {
  UartPacketType type;
  UartPayload payload;

  std::array<uint8_t, 5> get_raw();
};