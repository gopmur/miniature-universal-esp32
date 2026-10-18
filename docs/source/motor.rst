=========================
Motor Subsystem & Drivers
=========================

The FUMCARE-RP platform provides a robust, extensible motor control architecture designed to manage multiple CAN-based actuators simultaneously. It handles real-time control, hardware communication, timeout monitoring, and web-based telemetry broadcasting.

Architecture Overview
=====================

The motor subsystem is divided into three main layers:

* **Driver Layer:** A hardware-agnostic base class (``AbstractMotorDriver``) that manages CAN packet transmission, torque limits, motor direction, and timeout timers.
* **Task Layer:** A dedicated FreeRTOS thread (``MotorTask``) that manages the lifecycle, continuous torque application, and state monitoring of all instantiated motors.
* **HTTP/API Layer:** Exposes HTTP and WebSocket endpoints via the ``HttpMotorModule`` for zeroing positions, querying status, and streaming real-time alerts.

Hardware Abstraction: AbstractMotorDriver
=========================================

All motor drivers inherit from the ``AbstractMotorDriver`` class, which implements the ``AbstractCanDeviceReader`` interface to process incoming CAN packets.

Core Responsibilities
---------------------

* **Torque Clamping:** Ensures the calculated control torque never exceeds the defined ``max_torque`` limit for physical safety.
* **Direction Management:** Automatically inverts position, velocity, and torque values transparently if the motor is configured as ``BACKWARD``.
* **Timeout Monitoring:** Utilizes a ``MotorTimeoutTimer`` to transition the motor to a ``TIMEDOUT`` state if no CAN feedback is received within the configured ``CONFIG_HEXA_MOTOR_TIMEOUT_MS`` limit.

Supported Drivers
-----------------

The framework natively implements drivers for three standard robotic actuators:

* **ODrive:** Supports comprehensive CAN commands, including closed-loop control state switching and encoder estimates. It utilizes an internal semaphore to block initialization until valid position feedback is received.
* **SMC:** Custom packet formulation for SMC protocol motors, encapsulating specific float-to-uint torque conversions.
* **T-Motor:** Implements T-Motor specific CAN encoding bounds for torque, velocity, and positional feedback.

Extending the Driver
--------------------

To add a new motor type, inherit from ``AbstractMotorDriver`` and implement the required pure virtual packet generators:

.. code-block:: cpp

   virtual CanPacket make_torque_packet(float torque) = 0;
   virtual CanPacket make_read_encoder_packet() = 0;
   virtual CanPacket make_enable_packet() = 0;
   virtual CanPacket make_disable_packet() = 0;
   virtual CanPacket make_zero_pos_packet() = 0;

The Motor Task
==============

The ``MotorTask`` runs as a high-priority FreeRTOS thread and centrally manages up to 8 motors defined by ``CONFIG_HEXA_MOTOR_COUNT``.

* **Dynamic Initialization:** At boot, the task utilizes Boost preprocessor macros (``BOOST_PP_REPEAT``) to dynamically instantiate the correct motor driver classes (ODrive, SMC, or T-Motor) directly based on the user's Kconfig selections.
* **CAN Binding:** It maps each motor to the ``CanRecvTask``, ensuring that incoming CAN frames matching the motor's specific ID (or mask) are routed directly to the appropriate driver instance.
* **Control Loop:** It executes a continuous loop (10ms interval) that checks for state changes, broadcasts status updates via WebSockets, and dispatches the latest target torque commands to the active motors. If a motor is in a fault state, it automatically demands 0 torque.

HTTP & WebSocket APIs
=====================

The ``HttpMotorModule`` exposes endpoints to interact with the motor subsystem over the network:

* **GET /api/motor/zero-pos:** Triggers the ``zero_pos_all()`` function, resetting the internal encoder offset for all connected motors simultaneously.
* **GET /api/motor/status:** Returns a JSON array containing the CAN ID and current state (ok, timeout, uninitialized, can_error, or invalid) of every registered motor.
* **WS /api/motor/notification/status:** A dedicated WebSocket stream that pushes asynchronous JSON alerts to connected clients whenever a motor's status changes (e.g., dropping offline due to a CAN timeout).