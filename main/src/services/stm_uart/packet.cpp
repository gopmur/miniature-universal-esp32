#include "services/stm_uart/packet.hpp"

const char* get_contorl_mode_str(ControlMode mode) {
  switch (mode) {
    case ControlMode::AUTO:
      return "automatic";
    case ControlMode::SEMI_AUTO:
      return "semi-automatic";
    case ControlMode::MANUAL:
      return "manual";
    case ControlMode::SMART:
      return "smart";
  }
}