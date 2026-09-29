#include "controller/hexa.hpp"
#include "controller.hpp"

ControllerOutput HexaController::run(ControllerInput input) {
  ControllerOutput output;
  HexaControllerInput hexa_input = {
      .left_motor =
          {
              .position = input.motor_feedbacks[HEXA_LEFT_MOTOR_INDEX].position,
              .velocity = input.motor_feedbacks[HEXA_LEFT_MOTOR_INDEX].velocity,
          },
      .right_motor =
          {
              .position = input.motor_feedbacks[HEXA_RIGHT_MOTOR_INDEX].position,
              .velocity = input.motor_feedbacks[HEXA_RIGHT_MOTOR_INDEX].velocity,
          },
  };
  auto hexa_output = run(hexa_input);
  output.torques.fill(0);
  output.torques[HEXA_LEFT_MOTOR_INDEX] = hexa_output.left_motor.torque;
  output.torques[HEXA_RIGHT_MOTOR_INDEX] = hexa_output.right_motor.torque;
  return output;
}
