======================================
WebSocket Telemetry & Streaming Service
======================================

The FUMCARE-RP platform utilizes a dedicated high-performance WebSocket service (**WebSocketTask**) to stream real-time telemetry, system logs, and hardware states to connected clients. It operates as an independent FreeRTOS thread, ensuring that high-frequency data broadcasting does not block the main HTTP server or the core robotic control loops.

Architecture Overview
=====================

The WebSocket subsystem bridges the HTTP server and the internal RTOS tasks. 

* **Connection Handshake:** When a client requests a WebSocket upgrade via the **HttpStreamModule**, the HTTP server handles the handshake. Once successful, the socket file descriptor is passed to the **WebSocketTask** via the **add_connection** method.
* **Thread Suspension:** To conserve CPU cycles, the **WebSocketTask** remains suspended when there are zero active connections. It is woken up via **Sync::wait_for_notification()** the moment the first client connects.
* **Broadcast Loop:** While active, the task iterates through all open connections at a configured frequency, reads the latest system data, packages it into JSON, and dispatches the frames.

Supported Data Streams
======================

Clients can subscribe to specific data streams by connecting to their respective URI endpoints. The **WebSocketConnectionType** enum categorizes these streams:

* **IMU:** Streams 3-axis accelerometer and gyroscope (roll, pitch, yaw) data.
* **MOTOR:** Streams real-time kinematic data (position, velocity, torque) for all active motors.
* **MOTOR_STATUS:** An asynchronous event stream that pushes alerts only when a motor's operational status changes (e.g., fault, timeout, recovery).
* **TASK:** Streams FreeRTOS thread diagnostics, including CPU usage, minimum free stack, and task priorities for all active services.
* **HEAP:** Streams memory diagnostics, including total free heap, minimum free heap, and the largest available block.
* **OTA:** Streams progress updates during Over-The-Air firmware flashes.
* **SYS_LOG:** Streams the raw standard output console logs directly to the browser for wireless debugging.
* **LEGACY:** A binary-packed stream designed for backward compatibility with older control dashboards.

JSON Data Packaging
===================

Data is formatted into JSON objects before transmission. Every telemetry payload is automatically stamped with a highly accurate **microseconds** timestamp retrieved from the ESP timer. 

For example, the IMU packaging routine extracts the processed orientation and acceleration data from the **ImuTask** and formats it:

.. code-block:: cpp

   void WebSocketTask::fill_imu_data_json(JsonObject* imu_data_json) {
     add_time_stamp(imu_data_json);
     JsonObject accel_json;
     JsonObject gyro_json;
     gyro_json.set("x", imu_task->data.gyro.x);
     gyro_json.set("y", imu_task->data.gyro.y);
     gyro_json.set("z", imu_task->data.gyro.z);
     // ...
   }

Connection Management & Fault Tolerance
=======================================

The task maintains a thread-safe vector of active connections protected by a mutex. 

If a transmission fails (e.g., the client disconnects ungracefully or network memory is exhausted), the **handle_ws_transmit_status** method automatically flags the socket. If the status returns an **ESP_FAIL**, the connection is safely purged from the active list via **remove_connection**.

Kconfig Parameters
==================

The behavior and resource allocation of the WebSocket service can be tuned via the ESP-IDF menuconfig under **Tasks -> WebSocket**:

* **Priority:** The FreeRTOS task priority (default: 2).
* **Stack Size:** Memory allocated for JSON serialization and network broadcasting (default: 4096 bytes).
* **Frequency (Hz):** Dictates the update rate of the broadcast loop, defined by **CONFIG_HEXA_TASKS_WS_FREQ_HZ** (default: 33 Hz).