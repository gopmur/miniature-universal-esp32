=========================================
Motor Driver Stack and MotorTask Runtime
=========================================

The motor subsystem is the hardware-facing layer of the robot. It manages CAN bus
connections, driver initialization, torque limits, status tracking, and the
runtime motion commands that eventually reach the actuators.

Architecture
============

The project separates the system into a few layers:

* hardware driver abstraction
* motor runtime task
* HTTP and WebSocket management

The runtime is centered around ``MotorTask`` and the driver classes under the
custom driver stack.

Driver abstraction
==================

The motor drivers expose a common interface that abstracts the actual CAN packet
structure used by the device. This keeps the rest of the application independent
from the specific walker actuator hardware.

The base task API includes methods such as:

* ``set_torque(size_t motor_index, float torque)``
* ``enable(size_t motor_index)``
* ``disable(size_t motor_index)``
* ``zero_pos_all()``
* ``get_position(size_t motor_index)``
* ``get_velocity(size_t motor_index)``
* ``get_status(size_t motor_index)``
* ``get_torque_limit(size_t motor_index)``

The per-motor state is managed through the configured count and CAN identity.

Runtime behavior
================

``MotorTask`` loads its configuration from NVS, initializes each connected motor,
and then consumes control commands produced by the control layer. It is also the
source of the telemetry values used by the logger and the WebSocket broadcaster.

Important runtime features include:

* CAN ID and motor direction mapping
* torque constant and torque limit initialization
* per-motor status tracking
* zeroing and enable/disable operations

The runtime also exposes a standardized status model that is broadcast to the
frontend through the status WebSocket stream.

The motor subsystem's HTTP operations are documented in the
:doc:`Complete API Reference <api>`.

WebSocket status stream
======================

The motor status stream is event-driven: when a motor state changes, the
WebSocket task publishes the new status to the connected dashboard. See the
:doc:`Complete API Reference <api>` for the stream endpoint.

In practice, a browser client can subscribe to this endpoint to show live motor
health and fault events without polling the REST API.

Operational considerations
==========================

Because the motors are connected to the CAN bus and are safety-critical, the
software keeps tight control over values such as torque, timeout windows, and
direction inversion. The board should always be validated after changing the
motor configuration or ID mapping, because a single mismatched CAN mapping can
render the actuator control path dangerous or ineffective.
