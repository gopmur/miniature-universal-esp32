===========================================
Detailed REST & WebSocket API Documentation
===========================================

The FUMCARE-RP platform exposes a robust HTTP and WebSocket API routed through the **HttpRootModule** under the base **/api** path.

----------------
System Endpoints
----------------

GET /api/system/version
=======================
**Description:** Interfaces with the **esp_app_desc_t** system struct to retrieve compiled firmware metadata.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "version": "string",
       "idfVersion": "string",
       "buildDate": "string"
     }

GET /api/system/restart
=======================
**Description:** Safely triggers a soft reboot. The handler suspends the thread for 1,000 milliseconds using **Sync::sleep()** to allow the HTTP response to successfully dispatch to the client before invoking the native **esp_restart()** command.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

GET /api/system/factory-reset
=============================
**Description:** Performs a complete wipe of device storage. It calls **nvs_flash_erase()** to delete all saved controller calibrations, closes any active system logs, formats the SD card FAT partition via **esp_vfs_fat_sdcard_format()**, waits 1 second, and reboots.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

---------------
Wi-Fi Endpoints
---------------

GET /api/wifi/
==============
**Description:** Queries the native **esp_wifi_sta_get_ap_info** driver to check the current connection status of the ESP32. If connected, it extracts the BSSID array and converts it into a human-readable hexadecimal MAC address format.

* **Input Payload:** None.
* **Success Response (Connected):**

  .. code-block:: json

     {
       "connected": "boolean",
       "ssid": "string",
       "rssi": "number",
       "bssid": "string"
     }

  **Constraints:**
  * **connected:** Will be strictly `true`.
  * **bssid:** Must match the format "XX:XX:XX:XX:XX:XX".

* **Success Response (Disconnected):**

  .. code-block:: json

     {
       "connected": "boolean",
       "errorMessage": "string"
     }

GET /api/wifi/scan (Async)
==========================
**Description:** Triggers an active 802.11 network scan on a background thread to prevent blocking. It stops any ongoing scans, initiates a new active scan, allocates memory for the Access Point records, and evaluates the security auth mode of each discovered network.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     [
       {
         "rssi": "number",
         "bssid": "string",
         "ssid": "string",
         "open": "boolean"
       }
     ]

  **Constraints:**
  * **bssid:** Formatted as "XX:XX:XX:XX:XX:XX".

PUT /api/wifi/connect (Async)
=============================
**Description:** Disconnects from the current network and attempts to connect to a new wireless access point. It validates string lengths, configures the station struct, invokes **esp_wifi_connect()**, and blocks on a FreeRTOS Queue awaiting the connection result from the global **WifiEventCallback**.

* **Input Payload:**

  .. code-block:: json

     {
       "ssid": "string",
       "password": "string"
     }

  **Constraints:**
  * **ssid:** Maximum 31 characters.
  * **password:** Maximum 31 characters.

* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

GET /api/wifi/disconnect
========================
**Description:** Forces the ESP32 station interface to drop its current Wi-Fi connection.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

-----------------------------------
Over-The-Air (OTA) Update Endpoints
-----------------------------------

GET /api/update/check (Async)
=============================
**Description:** Opens an HTTPS client connection to the configured remote distribution server, fetches the raw JSON response containing the latest available version, and proxies it directly back to the frontend.

* **Input Payload:** None.
* **Success Response (200 OK):**
  Raw JSON object provided by the remote server (typically containing **api**: "string" and **core**: "string").

PUT /api/update/ (Async)
========================
**Description:** Locks the **ota_busy** flag, formats the URI for the target binary, begins the native OTA flash process, and continuously reports byte-download progress to the WebSocket task until the flash is complete.

* **Input Payload:**

  .. code-block:: json

     {
       "version": "string"
     }

* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

GET /api/update/status
======================
**Description:** Exposes the internal **ota_busy** state flag to determine if an OTA update is currently locking the system memory.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "busy": "boolean"
     }

---------------------
File System Endpoints
---------------------

PUT /api/fs/ls
==============
**Description:** Prepends **/sd** to the requested path, opens the directory using standard POSIX **opendir()**, and iterates through the entries to map out the folder structure.

* **Input Payload:**

  .. code-block:: json

     {
       "path": "string"
     }

  **Constraints:**
  * **path:** Cannot be empty. Must begin with the "/" character.

* **Success Response (200 OK):**

  .. code-block:: json

     {
       "files": [
         {
           "name": "string",
           "type": "string"
         }
       ]
     }

  **Constraints:**
  * **type:** Strictly constrained to "file", "directory", or "unsupported".

PUT /api/fs/cat (Async)
=======================
**Description:** Opens a file from the SD card in read mode and uses an asynchronous thread with **httpd_resp_send_chunk()** to stream the file in 32KB buffer blocks, preventing RAM exhaustion.

* **Input Payload:**

  .. code-block:: json

     {
       "path": "string"
     }

  **Constraints:**
  * **path:** Cannot be empty. Must begin with the "/" character.

* **Success Response (200 OK):**
  Raw binary or text file content.

---------------
Motor Endpoints
---------------

GET /api/motor/zero-pos
=======================
**Description:** Triggers the **zero_pos_all()** method on the MotorTask, instructing every active motor driver to record its current physical encoder reading as its new zero-offset reference point.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

GET /api/motor/status
=====================
**Description:** Iterates through the internal motor driver array to query the current hardware health state and CAN ID of all registered motors.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     [
       {
         "canId": "number",
         "status": "string"
       }
     ]

  **Constraints:**
  * **status:** Strictly constrained to "ok", "timeout", "uninitialized", "can_error", or "invalid".

-----------------
Logging Endpoints
-----------------

GET /api/log/session/start
==========================
**Description:** Wakes the suspended **LoggerTask**, triggering it to open **/sd/my_log.csv**, write the headers, and begin recording continuous motor and IMU telemetry at 100Hz.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

GET /api/log/session/stop
=========================
**Description:** Flips the **is_logging** flag to false, safely terminating the **LoggerTask** recording loop, flushing the file buffer, and closing the CSV.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

---------------------------
Control Subsystem Endpoints
---------------------------

GET /api/control/params
=======================
**Description:** Compiles the absolute master state of the robotic exoskeleton. It aggregates the global running boolean, formats the active mode enumeration into a string, and deeply queries the RAM structs of the manual, automatic, semiautomatic, and smart controllers to build a comprehensive system snapshot.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "running": "boolean",
       "mode": "string",
       "controlParams": {
         "manual": {
           "left": { "torque": "number" },
           "right": { "torque": "number" }
         },
         "automatic": {
           "left": { "torque": "number", "timeout": "number", "velocityThreshold": "number" },
           "right": { "torque": "number", "timeout": "number", "velocityThreshold": "number" }
         },
         "semiautomatic": {
           "weakLeg": "string",
           "startAssistAngle": "number",
           "stopAssistAngle": "number",
           "epsilon": "number",
           "left": { "torque": "number", "timeout": "number", "delay": "number" },
           "right": { "torque": "number", "timeout": "number", "delay": "number" }
         },
         "smart": {
           "left": { "torque": "number" },
           "right": { "torque": "number" }
         }
       }
     }

  **Constraints:**
  * **mode:** Strictly constrained to "manual", "automatic", "semi-automatic", or "smart".
  * **weakLeg:** Strictly constrained to "left" or "right".

PUT /api/control/start
======================
**Description:** Sets the ControlTask's **running** flag to true and broadcasts an **enable_all()** command to the MotorTask, energizing the physical actuators.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

PUT /api/control/stop
=====================
**Description:** Sets the ControlTask's **running** flag to false and broadcasts a **disable_all()** command to the MotorTask, forcing the robot into a safe, limp state.

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

PUT /api/control/set-mode/{mode}
================================
**Description:** Instantly switches the active kinematic controller. The ControlTask detects this state change on its next 10ms loop and automatically resets internal state timers to prevent erratic behavior.

* **Path Variable Constraints:**
  * **{mode}** must be strictly "manual", "automatic", "semiautomatic", or "smart".

* **Input Payload:** None.
* **Success Response (200 OK):**

  .. code-block:: json

     {
       "message": "string"
     }

Parameter Update Endpoints (Volatile RAM)
=========================================
**Description:** Updates the live target parameters for a specific controller. These values immediately alter physical behavior but are lost upon device reboot.

* **PUT /api/control/manual/params**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "left": { "torque": "number" },
         "right": { "torque": "number" }
       }

* **PUT /api/control/automatic/params**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "left": { "torque": "number", "timeout": "number" },
         "right": { "torque": "number", "timeout": "number" }
       }

* **PUT /api/control/semiautomatic/params**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "weakLeg": "string",
         "left": { "torque": "number", "timeout": "number", "delay": "number" },
         "right": { "torque": "number", "timeout": "number", "delay": "number" }
       }
  * **Constraints:** **weakLeg** must be "left" or "right".

* **PUT /api/control/smart/params**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "left": { "torque": "number" },
         "right": { "torque": "number" }
       }

Configuration Update Endpoints (Non-Volatile NVS)
=================================================
**Description:** Stores fundamental threshold settings permanently into the ESP32's flash memory via the custom **Nvs** driver, ensuring calibrations persist across power cycles.

* **PUT /api/control/automatic/configs**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "angleThreshold": "number",
         "left": { "velocityThreshold": "number" },
         "right": { "velocityThreshold": "number" }
       }

* **PUT /api/control/semiautomatic/configs**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "startAssistAngle": "number",
         "stopAssistAngle": "number",
         "epsilon": "number"
       }

* **PUT /api/control/smart/configs**
  * **Input Payload:** 
    
    .. code-block:: json

       {
         "velocityThreshold": "number"
       }

-----------------------------
Stream Endpoints (WebSockets)
-----------------------------

**Description:** These endpoints upgrade standard HTTP GET requests into persistent, bidirectional WebSockets. Once the handshake is complete, the **WebSocketTask** continuously formats internal structs into JSON frames and pushes them to the client.

* **WS /api/stream/imu:**

  .. code-block:: json

     {
       "microseconds": "number",
       "accel": { "x": "number", "y": "number", "z": "number" },
       "gyro": { "x": "number", "y": "number", "z": "number" }
     }

* **WS /api/stream/motor:**

  .. code-block:: json

     {
       "microseconds": "number",
       "feedbacks": [
         {
           "position": "number",
           "velocity": "number",
           "torque": "number"
         }
       ]
     }

* **WS /api/stream/tasks:**

  .. code-block:: json

     {
       "microseconds": "number",
       "TaskName": {
         "cpu": "number",
         "minFreeStack": "number",
         "currentPriority": "number",
         "basePriority": "number",
         "state": "string",
         "stackSize": "number (optional)"
       }
     }

* **WS /api/stream/heap:**

  .. code-block:: json

     {
       "microseconds": "number",
       "free": "number",
       "largestBlock": "number",
       "minFree": "number",
       "totalSize": "number"
     }

* **WS /api/stream/ota:**

  .. code-block:: json

     {
       "total": "number",
       "progress": "number"
     }

  *(Upon completion, sends:)*

  .. code-block:: json

     {
       "ok": "boolean"
     }

* **WS /api/motor/notification/status:**
  Pushed asynchronously only on state change:

  .. code-block:: json

     {
       "motorId": "number",
       "motorIndex": "number",
       "status": "string"
     }

* **WS /api/log/system:**
  Pushes raw string data intercepted directly from the ESP-IDF **vprintf** console log buffer.

* **WS /api/control/manual/torque:**
  **Bidirectional:** Client sends the following to instantly mutate the manual controller RAM struct without HTTP overhead:

  .. code-block:: json

     {
       "leftTorque": "number",
       "rightTorque": "number"
     }

--------------------------
Legacy Dashboard Endpoints
--------------------------

WS /api/legacy/
===============
**Description:** A WebSocket endpoint to maintain compatibility with older dashboards. It parses a JSON **action** string and mutates the modern system state to match.

* **Input Payload Structure:**

  .. code-block:: json

     {
       "action": "string",
       "mode": "number (optional)",
       "weak_leg": "number (optional)",
       "left_assistance_value": "number (optional)",
       "right_assistance_value": "number (optional)",
       "left_assistance_time": "number (optional)",
       "right_assistance_time": "number (optional)",
       "left_delay": "number (optional)",
       "right_delay": "number (optional)"
     }

* **Output Payload (Binary):** A packed C-struct **LegacyDataPacket** containing 9 floats/ints representing legacy status, action, and motor positions.