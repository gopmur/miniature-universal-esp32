====================================
Real-time Control Subsystem
====================================

The control subsystem is the decision layer that translates raw feedback from the
IMU and motor drivers into torque commands for the robot. Its implementation is
centered around ``ControlTask`` and the four control modes defined in
``main/inc/tasks/control.hpp``.

Control task lifecycle
======================

``ControlTask`` derives from the project thread base and owns the active control
state:

* ``running``
* ``control_mode``
* ``prev_control_mod``
* the ``hexa_controller`` group containing the active controller instances

The task is responsible for:

* switching between manual and automatic modes
* applying torque to the motors
* loading and saving configuration values
* distributing the desired control state through the HTTP and WebSocket APIs

The controller set is composed of:

* ``ZeroController``
* ``HexaManualController``
* ``HexaAutomaticController``
* ``HexaSemiautomaticController``
* ``HexaSmartController``

This structure allows the same task to share configuration and state while the
active mode changes at runtime.

Controller modes
================

Zero controller
---------------

The zero controller is the fallback mode. It outputs zero torque for every motor,
which keeps the system stable when the controller is disabled or an invalid mode is
selected.

Manual controller
-----------------

The manual controller is literal: it takes direct torque requests and applies them
without higher-level gait logic.

Manual torque updates are available through the WebSocket interface. See the
:doc:`Complete API Reference <api>` for the endpoint and payload details.

Automatic controller
--------------------

The automatic controller performs the core gait logic. It makes decisions using
signals from the motion state and threshold values such as angle and velocity.
The configuration values are loaded from NVS-backed settings and exposed by the
HTTP routes.

Key fields in the automatic controller configuration include:

* ``angleThreshold``
* ``left.velocityThreshold``
* ``right.velocityThreshold``
* per-leg torque and timeout values

These settings can be persisted through the control API; see the
:doc:`Complete API Reference <api>` for the endpoint details.

Semiautomatic controller
------------------------

The semiautomatic controller is intended for assisted motion where one side is
weak and needs support. Each leg can be configured with:

* torque
* timeout
* delay
* a weak-leg selection

The request payload uses ``weakLeg`` with values ``left`` or ``right`` and a
per-leg object with the relevant fields.

Smart controller
---------------

The smart controller evaluates the motion state and applies its own internal
logic. It also includes timeout and delay-based state protection to avoid rapid
oscillation or simultaneous leg activation.

Control HTTP routes are implemented by ``HttpControlModule``. For the available
operations, request payloads, and response formats, see the
:doc:`Complete API Reference <api>`.

WebSocket manual torque stream
=============================

The manual torque stream is registered with ``register_ws_uri`` and updates
torque while a client remains connected. See the
:doc:`Complete API Reference <api>` for its endpoint and payload details.

It is intentionally lightweight and supports interactive manual-drive control in
the browser or a control dashboard.
