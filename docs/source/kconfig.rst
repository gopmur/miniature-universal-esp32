===========================================
Project Configuration and ESP-IDF Kconfig
===========================================

The project uses the standard ESP-IDF Kconfig system with the top-level menu
``HEXA``. The menu is assembled with ``rsource`` entries in
``main/Kconfig`` and includes the following sections:

* motors
* IMU
* CAN bus
* HTTP
* OTA
* Wi-Fi
* tasks
* SD card

Accessing the menu
==================

Run:

.. code-block:: bash

   idf.py menuconfig

Then open the ``HEXA`` menu to adjust board and runtime parameters. The config is
used at compile time and at runtime through the generated compile-time macros.

HTTP configuration
==================

The HTTP subsystem is configured in ``main/config/Kconfig.http``.

Relevant options:

* ``CONFIG_HEXA_HTTP_USE_HTTPS``
* ``CONFIG_HEXA_HTTP_MAX_REQUEST_LEN``
* ``CONFIG_HEXA_HTTP_ALLOW_CORS``
* ``CONFIG_HEXA_HTTP_KEEP_ALIVE``
* ``CONFIG_HEXA_HTTP_MAX_OPEN_SOCKET``
* ``CONFIG_HEXA_HTTP_MAX_URI_HANDLER_COUNT``

These values control the HTTP server behavior, request limits, route capacity,
connectivity, and browser compatibility.

The defaults currently used by the project are:

* HTTPS: enabled
* max request length: 512 bytes
* CORS: disabled by default
* keep-alive: enabled
* max sockets: 16
* max URI handlers: 128

OTA configuration
=================

The OTA system is configured by:

* ``CONFIG_HEXA_OTA_SERVER_ADDRESS``

This is the address used by the firmware update client to query the remote
release server and fetch the correct image binary.

Wi-Fi configuration
===================

The Wi-Fi menu controls the station and access point behavior used by the board.
This is the core network layer for the smart device and is used by the HTTP,
OTA and dashboard flows.

Task configuration
==================

The task section contains the runtime scheduling properties of the background
workers, including:

* DNS task
* WebSocket task
* IMU task
* control task
* logger task

The project relies on these values for task priorities, stack sizes, and loop
frequencies. The scheduler uses these values to keep the high-priority control
loops responsive and the telemetry updates smooth.

Motor configuration
===================

The motor system configuration includes actuator parameters and bus-related
settings such as:

* power-enable pin
* timeout delay
* motor count
* per-motor CAN ID
* per-motor direction inversion
* driver selection

This determines how the CAN-based actuator stack is initialized and how many
motors participate in control and status streaming.

IMU configuration
=================

The IMU menu configures the I2C wiring and the sampling behavior. The main values
are the SDA and SCL pins and the IMU polling frequency.

SD-card configuration
=====================

The SD menu configures the SPI pins used by the card reader and therefore the
storage location for logs and generated web assets.

Practical guidance
==================

When adjusting configuration, the safest workflow is:

1. change the relevant Kconfig value
2. regenerate the build
3. verify the subsystem boot sequence
4. confirm that route registration and task behavior remain consistent

Kconfig values affect the firmware binary itself; after changing a critical
setting, rebuild and reflash the board before testing the affected subsystem.
