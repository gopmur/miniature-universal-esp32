#pragma once

#include "controller.hpp"

class ZeroController : public Controller {
  public:
  ControllerOutput run(ControllerInput input);
};