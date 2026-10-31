===========================================
API Reference
===========================================

This page documents the public HTTP API exposed by the project. Every HTTP route
is mounted under the root module named ``api`` and all route names below are
shown in their fully-qualified form.

Type conventions used in this document
=====================================

The examples below intentionally use meaningful JSON values rather than abstract
placeholder types so they remain valid snippet examples for readers and editors.
This makes the documentation readable in source form and avoids confusing
syntax-highlighting tools when a snippet is copied into templates or test data.

Common HTTP response semantics
=============================

Routes that use the normal ``HttpModule`` middleware can return these standard
HTTP behaviors:

* ``HTTP 200`` for successful processing
* ``HTTP 400`` when JSON parsing or validation fails
* ``HTTP 413`` when the request body exceeds ``CONFIG_HEXA_HTTP_MAX_REQUEST_LEN``
* ``HTTP 500`` when an async or operational task fails unexpectedly
* ``OPTIONS`` preflight responses for routes registered with the ``*_with_option``
  registration variants

Most responses in this project are JSON objects or JSON arrays. A few endpoints
stream raw bytes or text instead of returning a JSON envelope.

System routes
=============

GET /api/system/version
-----------------------

Returns the firmware metadata from ``esp_app_get_description()``.

Success response:

.. code-block:: json

   {
     "version": "1.2.3",
     "idfVersion": "v5.3.2",
     "buildDate": "2026-10-31"
   }

Field definitions:

* ``version``: application version string
* ``idfVersion``: ESP-IDF version reported by the firmware metadata
* ``buildDate``: build date reported by the firmware metadata

GET /api/system/restart
-----------------------

Triggers a graceful device restart. The server sends a JSON message first, then
waits one second before calling ``esp_restart()``.

Success response:

.. code-block:: json

   {
     "message": "restarting in 1 seconds"
   }

Field definitions:

* ``message``: restart notice string, typically ``"restarting in 1 seconds"``

GET /api/system/factory-reset
-----------------------------

Erases the non-volatile storage, closes the active system log file, formats the
SD card, and then reboots the device.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: status string; in the successful path the implementation sets a
  restart notice message before rebooting

Failure response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: error text, for example ``"failed to format sd card"``

Wi-Fi routes
============

GET /api/wifi/
--------------

Returns the current station connection state. If the device is connected, it
includes the AP SSID, RSSI and BSSID. If not connected, it includes a structured
error description instead.

Connected success response:

.. code-block:: json

   {
     "connected": true,
     "ssid": "MyNetwork",
     "rssi": -55,
     "bssid": "AA:BB:CC:DD:EE:FF"
   }

Disconnected success response:

.. code-block:: json

   {
     "connected": true,
     "errorMessage": "not connected"
   }

Field definitions:

* ``connected``: ``true`` when a station connection exists, otherwise ``false``
* ``ssid``: current AP SSID when connected
* ``rssi``: received signal strength indicator in dBm when connected
* ``bssid``: current AP MAC address as ``XX:XX:XX:XX:XX:XX``
* ``errorMessage``: textual reason when disconnected; examples include
  ``"not connected"`` and ``"wifi not initialized"``

GET /api/wifi/scan
------------------

Starts a Wi-Fi scan on a background thread and returns the discovered access
points.

Success response:

.. code-block:: json

   [
     {
       "rssi": -55,
       "bssid": "AA:BB:CC:DD:EE:FF",
       "ssid": "MyNetwork",
       "open": true
     }
   ]

Field definitions:

* ``rssi``: received signal strength in dBm
* ``bssid``: access-point MAC address as ``XX:XX:XX:XX:XX:XX``
* ``ssid``: access-point SSID string
* ``open``: ``true`` when the AP uses an open authentication mode, otherwise
  ``false``

PUT /api/wifi/connect
---------------------

Connects the ESP32 to a new AP using the supplied station configuration.

Request body:

.. code-block:: json

   {
     "ssid": "MyNetwork",
     "password": "super-secret-password"
   }

Field definitions:

* ``ssid``: Wi-Fi network name
* ``password``: Wi-Fi password

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

Validation and failure response:

.. code-block:: json

   {
     "ssid": "MyNetwork",
     "password": "password is too short"
   }

Field definitions:

* ``ssid``: validation error for an invalid or too-long SSID
* ``password``: validation error for an invalid or too-long password

Operational error responses are also possible and are returned as a message
string:

.. code-block:: json

   {
     "message": "connection failed"
   }

Examples of operational errors include:

* ``"connection failed"``
* ``"ap not found"``
* ``"unhandled error"``

GET /api/wifi/disconnect
------------------------

Disconnects the ESP32 from the currently-associated access point.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

OTA routes
==========

GET /api/update/check
---------------------

Starts an asynchronous update-check task. The route does not enforce a fixed
JSON schema; it returns the raw JSON payload from the remote firmware server.

Success response:

.. code-block:: json

   {
     "<server-defined-field>": <server-defined-value>
   }

Field definitions:

* The exact field names and values are defined by the remote firmware distribution
  server and are not fixed by the ESP32 firmware.
* The returned value is an object, not a primitive array.

PUT /api/update/
----------------

Starts a firmware update using HTTPS OTA.

Request body:

.. code-block:: json

   {
     "version": "1.2.3"
   }

Field definitions:

* ``version``: target firmware version to download and install

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

Failure response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: error text generated by the OTA task, for example a certificate
  or download failure description

GET /api/update/status
----------------------

Returns the current OTA state.

Success response:

.. code-block:: json

   {
     "busy": true
   }

Field definitions:

* ``busy``: ``true`` while an OTA update is running, otherwise ``false``

File-system routes
==================

PUT /api/fs/ls
--------------

Lists the contents of the directory rooted under ``/sd``.

Request body:

.. code-block:: json

   {
     "path": "/logs"
   }

Field definitions:

* ``path``: absolute path on the SD card, for example ``"/logs"``

Success response:

.. code-block:: json

   {
     "files": [
       {
         "name": "system.log",
         "type": "file"
       },
       {
         "name": "config",
         "type": "directory"
       }
     ]
   }

Field definitions:

* ``files``: array of directory entries
* ``name``: entry name as reported by the OS
* ``type``: entry type string; the implementation returns ``"file"``,
  ``"directory"``, or ``"unsupported"``

Failure response:

.. code-block:: json

   {
     "path": "/logs"
   }

Field definitions:

* ``path``: validation error; currently either ``"cannot be empty"`` or
  ``"should start with /"``

PUT /api/fs/cat
---------------

Streams a file from the SD card in chunks and sends the raw file bytes back to
the client.

Request body:

.. code-block:: json

   {
     "path": "/logs"
   }

Field definitions:

* ``path``: absolute file path on the SD card, for example ``"/logs/system.log"``

Success response:

This route does not return a JSON object. It sends the file content as a raw
payload, chunked by the HTTP server, and ends with a zero-length chunk.

Failure response:

.. code-block:: json

   {
     "path": "/logs"
   }

Field definitions:

* ``path``: validation or file-not-found reason; examples include
  ``"cannot be empty"``, ``"should start with /"``, and ``"does not exist"``

Motor routes
============

GET /api/motor/zero-pos
-----------------------

Resets the motor encoder offsets for all configured motors.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

GET /api/motor/status
---------------------

Returns the current runtime status for each configured motor.

Success response:

.. code-block:: json

   [
     {
       "canId": 1,
       "status": "ok"
     },
     {
       "canId": 2,
       "status": "timeout"
     }
   ]

Field definitions:

* ``canId``: numeric CAN identifier for that motor
* ``status``: one of ``"ok"``, ``"timeout"``, ``"uninitialized"``,
  ``"can_error"``, or ``"invalid"``

GET /api/motor/
---------------

Returns per-motor configuration metadata.

Success response:

.. code-block:: json

   [
     {
       "canId": 1,
       "torqueLimit": 0.75,
       "torqueConstant": 0.35,
       "direction": "forward"
     }
   ]

Field definitions:

* ``canId``: numeric CAN identifier for the motor
* ``torqueLimit``: configured torque limit for the motor
* ``torqueConstant``: configured torque constant for the motor
* ``direction``: ``"forward"`` or ``"backward"``

PUT /api/motor/config
---------------------

Updates one or more per-motor configuration values.

Request body:

.. code-block:: json

   {
     "index": 1,
     "torqueConstant": 0.35,
     "torqueLimit": 0.75
   }

The implementation updates whichever fields are present; omitted values are left
unchanged.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

Failure response:

.. code-block:: json

   {
     "torqueLimit": "invalid value"
   }

Field definitions:

* ``<field>``: validation error name for a malformed field, for example
  ``"torqueLimit"`` or ``"torqueConstant"``

Control routes
==============

GET /api/control/params
-----------------------

Returns the entire active control state in a single object.

Success response:

.. code-block:: json

   {
     "running": true,
     "mode": "manual",
     "controlParams": {
       "manual": {
         "left": {
           "torque": 0.2
         },
         "right": {
           "torque": 0.25
         }
       },
       "automatic": {
         "left": {
           "torque": 0.35,
           "timeout": 2.5,
           "velocityThreshold": 0.8
         },
         "right": {
           "torque": 0.3,
           "timeout": 2.2,
           "velocityThreshold": 0.75
         },
         "angleThreshold": 18.0
       },
       "semiautomatic": {
         "weakLeg": "left",
         "left": {
           "torque": 0.4,
           "timeout": 1.8,
           "delay": 0.6
         },
         "right": {
           "torque": 0.38,
           "timeout": 1.7,
           "delay": 0.55
         },
         "startAssistAngle": 12.0,
         "stopAssistAngle": 20.0,
         "epsilon": 0.1
       },
       "smart": {
         "left": {
           "torque": 0.32,
           "timeout": 1.9,
           "delayTimeout": 0.7
         },
         "right": {
           "torque": 0.29,
           "timeout": 1.8,
           "delayTimeout": 0.65
         }
       }
     }
   }

Field definitions:

* ``running``: ``true`` when the control task is active, otherwise ``false``
* ``mode``: current control mode value
* ``controlParams.manual.left.torque``: left-leg manual drive torque
* ``controlParams.manual.right.torque``: right-leg manual drive torque
* ``controlParams.automatic.left.torque``: left automatic torque value
* ``controlParams.automatic.left.timeout``: left automatic timeout value
* ``controlParams.automatic.left.velocityThreshold``: left automatic velocity
  threshold
* ``controlParams.automatic.right.torque``: right automatic torque value
* ``controlParams.automatic.right.timeout``: right automatic timeout value
* ``controlParams.automatic.right.velocityThreshold``: right automatic velocity
  threshold
* ``controlParams.automatic.angleThreshold``: global automatic angle threshold
* ``controlParams.semiautomatic.weakLeg``: leg currently marked as weak,
  ``"left"`` or ``"right"``
* ``controlParams.semiautomatic.left.*`` and
  ``controlParams.semiautomatic.right.*``: semiautomatic per-leg torque, timeout,
  and delay values
* ``controlParams.semiautomatic.startAssistAngle``: semiautomatic assist start
  threshold
* ``controlParams.semiautomatic.stopAssistAngle``: semiautomatic assist stop
  threshold
* ``controlParams.semiautomatic.epsilon``: semiautomatic epsilon threshold
* ``controlParams.smart.left.*`` and ``controlParams.smart.right.*``: smart-mode
  torque and timing values

PUT /api/control/start
----------------------

Starts the controller and enables all motors.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

PUT /api/control/stop
---------------------

Stops the controller and disables all motors.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

PUT /api/control/set-mode/manual
--------------------------------

Switches the controller to manual mode.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

PUT /api/control/set-mode/automatic
----------------------------------

Switches the controller to automatic mode.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

PUT /api/control/set-mode/semiautomatic
---------------------------------------

Switches the controller to semiautomatic mode.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

PUT /api/control/set-mode/smart
-------------------------------

Switches the controller to smart mode.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

PUT /api/control/manual/params
-----------------------------

Updates the manual torque values for both legs.

Request body:

.. code-block:: json

   {
     "left": {
       "torque": 0.2
     },
     "right": {
       "torque": 0.3
     }
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "torqueLimit": "invalid value"
   }

PUT /api/control/automatic/params
---------------------------------

Updates the automatic-mode left and right torque and timeout values.

Request body:

.. code-block:: json

   {
     "left": {
       "torque": 0.35,
       "timeout": 2.5
     },
     "right": {
       "torque": 0.3,
       "timeout": 2.2
     }
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "timeout": "out of range"
   }

PUT /api/control/automatic/configs
----------------------------------

Updates the automatic controller thresholds.

Request body:

.. code-block:: json

   {
     "angleThreshold": 18.0,
     "left": {
       "velocityThreshold": 0.8
     },
     "right": {
       "velocityThreshold": 0.75
     }
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "angleThreshold": "invalid value"
   }

PUT /api/control/semiautomatic/params
------------------------------------

Updates semiautomatic drive parameters.

Request body:

.. code-block:: json

   {
     "weakLeg": "left",
     "left": {
       "torque": 0.4,
       "timeout": 1.8,
       "delay": 0.6
     },
     "right": {
       "torque": 0.38,
       "timeout": 1.7,
       "delay": 0.55
     }
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "weakLeg": "invalid value"
   }

PUT /api/control/semiautomatic/configs
--------------------------------------

Updates semiautomatic threshold settings.

Request body:

.. code-block:: json

   {
     "startAssistAngle": 12.0,
     "stopAssistAngle": 20.0,
     "epsilon": 0.1
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "epsilon": "invalid value"
   }

PUT /api/control/smart/params
-----------------------------

Updates smart-mode values for both legs.

Request body:

.. code-block:: json

   {
     "left": {
       "torque": 0.32
     },
     "right": {
       "torque": 0.29
     }
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "left": "invalid value"
   }

PUT /api/control/smart/configs
------------------------------

Updates smart-mode configuration.

Request body:

.. code-block:: json

   {
     "velocityThreshold": 0.75
   }

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Failure response:

.. code-block:: json

   {
     "velocityThreshold": "invalid value"
   }

Logger routes
=============

GET /api/log/session/start
--------------------------

Starts a new CSV logging session.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

GET /api/log/session/stop
-------------------------

Stops the active CSV logging session.

Success response:

.. code-block:: json

   {
     "message": "success"
   }

Field definitions:

* ``message``: success message string, typically ``"success"``

WebSocket routes
===============

The project also registers several WebSocket endpoints. These routes do not use
HTTP JSON response envelopes; instead, they maintain a socket connection and push
JSON frames over the connection.

Endpoint list:

* ``WS /api/stream/imu``
* ``WS /api/stream/motor``
* ``WS /api/stream/tasks``
* ``WS /api/stream/ota``
* ``WS /api/stream/heap``
* ``WS /api/log/system``
* ``WS /api/control/manual/torque``
* ``WS /api/motor/notification/status``

The WebSocket payloads are implementation-defined per stream, but the common
shape is represented by a timestamped payload such as:

.. code-block:: json

   {
     "microseconds": 123456,
     "accel": {
       "x": 0.12,
       "y": -0.08,
       "z": 0.94
     }
   }

The exact fields depend on the stream; for example:

* IMU payload includes ``accel`` and ``gyro`` objects
* OTA payload includes progress-related fields
* log payload includes a log line or log event object
* status payload includes motor state information

The project does not publish a single fixed schema for all WebSocket streams;
stream-specific payloads are determined by the broadcaster implementation. A
representative example is:

.. code-block:: json

   {
     "microseconds": 123456,
     "accel": {
       "x": 0.12,
       "y": -0.08,
       "z": 0.94
     },
     "gyro": {
       "x": 1.2,
       "y": -0.7,
       "z": 2.4
     }
   }
