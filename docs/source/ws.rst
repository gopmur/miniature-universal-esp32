=========================
WebSocket Telemetry Streams
=========================

The WebSocket subsystem is implemented by ``WebSocketTask`` and provides a
real-time data path for telemetry and status updates. It is separate from the
main HTTP thread and keeps socket traffic decoupled from the control loop.

Connection model
================

The task maintains a list of active connections, each represented by:

.. code-block:: cpp

   struct WebSocketConnection {
     int fd;
     WebSocketConnectionType stream;
   };

The stream type is an enum with entries such as:

* ``TASK``
* ``IMU``
* ``MOTOR``
* ``OTA``
* ``SYS_LOG``
* ``LEGACY``
* ``HEAP``
* ``MOTOR_STATUS``

The WebSocket task is the single broadcaster for these streams, which means each
connected client receives only the data relevant to its subscribed stream.

Stream registration
===================

The HTTP module creates the WebSocket routes with
``register_ws_uri()``. See the :doc:`Complete API Reference <api>` for the
complete endpoint list.

The common pattern is:

1. client connects
2. handshake completes
3. application callback registers the socket to a specific stream type
4. task pushes JSON data as frames at runtime

Telemetry payloads
==================

Each stream produces structured JSON data. For example, the IMU stream contains
position and acceleration payloads, while the OTA stream contains progress values.

A typical telemetry frame has a timestamp field and a stream-specific payload,
for example:

.. code-block:: json

   {
     "microseconds": 123456789,
     "accel": {"x": 0.2, "y": 0.0, "z": 0.9},
     "gyro": {"x": 5.0, "y": 12.0, "z": -3.0}
   }

The WebSocket task is also responsible for checking socket health, removing dead
connections, and sending keepalive or ping messages when appropriate.

High-level responsibilities
=========================

The task is designed to be a lightweight, high-frequency broadcaster. It is used
for:

* live motor telemetry
* IMU sensor telemetry
* OTA update progress
* system logs
* heap and task runtime status
* motor state-change notifications

This separation keeps the HTTP server free from blocking pushes and ensures that
web clients receive fast updates without interrupting the core control loop.
