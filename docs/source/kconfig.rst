=============================
Project Configuration
=============================

The FUMCARE-RP platform utilizes the standard ESP-IDF ``Kconfig`` system for compile-time configuration. This allows you to easily modify hardware pins, network credentials, task priorities, and motor properties without directly editing the C++ source code.

To access the configuration menu, ensure your ESP-IDF environment is activated and run the following command in your terminal:

.. code-block:: bash

   idf.py menuconfig

Navigate to the **HEXA** menu to configure the platform-specific subsystems.

Hardware Subsystems
===================

Motors
------
Configure the robotic actuators connected to the CAN bus.

* **Power enable pin:** The GPIO pin used to toggle power to the motor drivers.
* **Timeout delay ms:** The allowed communication timeout threshold (default: 50 ms).
* **Number of motors:** The platform supports up to 8 simultaneous motors.

For each active motor slot (Motor 0 to 7), you can individually configure:

* **ID:** The CAN node ID (0x0 to 0x7FF).
* **Type:** The motor driver protocol to use. Supported drivers are **ODrive**, **SMC**, and **T-Motor**.
* **Receive ID:** The expected response ID (specifically required for T-Motor protocols).
* **Is backwards:** A boolean flag to invert the motor's kinematic direction.

IMU (Inertial Measurement Unit)
-------------------------------
Configures the I2C bus used to communicate with the onboard IMU.

* **SDA pin:** I2C Serial Data pin (default: 21).
* **SCL pin:** I2C Serial Clock pin (default: 22).

CAN Bus
-------
Configures the Two-Wire Automotive Interface (TWAI) used for high-speed hardware communication.

* **Rx Pin / Tx Pin:** GPIO pins for the CAN transceiver (defaults: Rx=26, Tx=25).
* **Baudrate (KHz):** CAN bus speed (default: 1000 KHz).
* **Transmit Queue Len:** Size of the transmission buffer (default: 8).

SD Card
-------
Configures the SPI bus used for the MicroSD card reader, which holds web assets and logs.

* **CS pin:** default 5
* **MOSI pin:** default 23
* **MISO pin:** default 19
* **SCLK pin:** default 18

Software & Network Configuration
================================

Wi-Fi
-----
* **SSID & Password:** Default credentials for the ESP32's Access Point or Station mode.
* **Channels:** Wi-Fi channel allocation (default: 6).
* **Max Connections:** Maximum allowed simultaneous client connections (default: 3).

HTTP Server
-----------
Configures the bounds and security of the embedded web server.

* **Use HTTPS:** Toggles SSL/TLS encryption (default: true).
* **Max request length:** Maximum allowed payload size in bytes (default: 512).
* **Allow CORS:** Enables Cross-Origin Resource Sharing for remote frontend development.
* **Enable keep alive:** Keeps TCP connections open for subsequent requests to reduce latency.
* **Max open sockets & URI handlers:** Resource allocation limits for the web server (defaults: 16 sockets, 128 handlers).

OTA (Over-The-Air Updates)
--------------------------
* **Server Address:** The IP address and port of the firmware distribution server (default: ``192.168.4.2:3000``).

RTOS Tasks
==========
Configure the FreeRTOS task priorities, stack sizes, and execution frequencies for the core software loops.

* **DNS Task:** Handles local name resolution. You can customize the ``Domain Name`` here (default: ``hexa.lan``).
* **WebSocket Task:** Streams live telemetry and handles real-time control (default frequency: 33 Hz).
* **IMU Task:** Reads and processes sensor data (default frequency: 200 Hz).
* **Control Task:** Executes the main robotic control loop and kinematic calculations.