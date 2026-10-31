===============================
Getting Started
===============================

Welcome to the **FUMCARE Robotics Platform (FUMCARE-RP)**. 

FUMCARE-RP is a framework designed to drastically simplify the software development of modern robots that rely on the **CAN bus** as their primary protocol for communicating with actuators and sensors. 

It is natively built for the **mini-universal board** featuring a single **ESP32-C5** processor, and is architected to be highly portable to future ESP-IDF compatible microcontrollers. FUMCARE-RP abstracts away the boilerplate of networking, storage, logging, and hardware control so you can focus entirely on robotic behavior and kinematics.

Key Features
============

* **CAN-Centric Architecture:** Optimized for robust, high-speed communication with CAN-based robotic actuators and sensors.
* **Advanced Web Server:** A modular HTTP server that automatically serves static files using your project's directory structure to generate URIs. It also supports dynamic endpoints for controlling the platform, natively handling both simple synchronous and asynchronous requests.
* **Universal Motor Abstraction:** Simultaneously manage up to **8 motors**. The platform allows you to mix and match different motor drivers and configurations at compile-time using **Kconfig**.
* **Integrated IMU:** Native support for reading and processing data from the board's internal Inertial Measurement Unit.
* **SD Card Storage:** Reliable non-volatile storage using an SD card for configurations, static web assets, and data persistence.
* **Tri-Output System Logger:** An automatic, robust logging subsystem that simultaneously writes to:
  
  1. The standard UART console.
  2. A dedicated WebSocket system log endpoint for wireless debugging.
  3. Persistent log files on the SD card.

* **Real-time Task Monitoring:** Built-in WebSocket endpoints for live monitoring of RTOS tasks and system health.
* **Integrated DNS Server:** IP address discovery using a native DNS server directly on the ESP32.

Prerequisites
=============

1. **Hardware:** The FUMCARE mini-universal board (ESP32-C5) and a formatted MicroSD card.
2. **Build tools:** The Espressif IoT Development Framework ESP-IDF v6.0.5, Node.js with bun or npm, and Python 3.

Quick Start
===========

1. Get the Source
-----------------

Clone the FUMCARE-RP repository into your workspace:

.. code-block:: bash

   git clone https://github.com/gopmur/miniature-universal-esp32.git
   cd miniature-universal-esp32
   git submodule update --init --recursive

2. Configure the Target
-----------------------

Set the compiler to Clang

.. tab-set::

   .. tab-item:: Linux/macOS
      :sync: unix

      .. code-block:: bash

         export IDF_TOOLCHAIN=clang
      
   .. tab-item:: Windows (Powershell)
      :sync: win

      .. code-block:: powershell 
         
         $env:IDF_TOOLCHAIN = clang
   
   .. tab-item:: Windows (CMD)
      :sync: win

      .. code-block:: bat
         
         set IDF_TOOLCHAIN=clang

Set your ESP-IDF environment to target the ESP32-C5 (or your current ESP-IDF compatible processor):

.. code-block:: bash

   idf.py set-target esp32c5

Open the project configuration menu. Here, you can configure your Wi-Fi credentials, SD card pins, CAN bus settings, and motor driver defaults:

.. code-block:: bash

   idf.py menuconfig

Configure API address for the dashboard by first creating the config file and then setting your desired addresses
in it.

.. code-block:: bash

   cp app/hexa-webapp/config.sample.ts app/hexa-webapp/config.ts

Open /app/hexa-webapp/config.ts and configure the addresses. The default value should work for the firmware built with
HTTPS disabled.

.. code-block:: javascript
   
   export const API_ADDRESS = "http://192.168.4.1"
   export const WS_API_ADDRESS = "ws://192.168.4.1"

3. Build and Flash
------------------

First, build the frontend web application so its static files are available in the ``dist`` directory:

.. code-block:: bash

   cd app/hexa-webapp
   bun install
   bun run build
   cd ../..

Alternatively, use npm:

.. code-block:: bash

   cd app/hexa-webapp
   npm install
   npm run build
   cd ../..

Next, generate the C/C++ assets for the static HTTP files using the provided Python script:

.. code-block:: bash

   python main/scripts/generate_http_assets.py app/hexa-webapp/dist generated

This script will compress the web assets and place them into the ``generated/assets`` directory. It will then generate the ``generated/inc/http_assets.hpp`` header file, which includes the necessary code to automatically bind these assets to URIs generated from the folder's tree structure.

Finally, build and flash the firmware to your board using ESP-IDF:

.. code-block:: bash

   idf.py build flash

To open the serial monitor after flashing and observe the output, run:

.. code-block:: bash

   idf.py monitor

Exploring the Subsystems
========================

Once the board successfully boots and mounts the SD card, the core services will start automatically.

* **Web & APIs:** Drop your static HTML/JS/CSS files into the designated web directory on the SD card; the server will automatically route them. You can bind custom C/C++ handlers to dynamic endpoints for robotic control.
* **Wireless Debugging:** Connect a WebSocket client to the FUMCARE-RP IP address to stream logs wirelessly and monitor RTOS task execution times without needing a serial cable.
* **Motor Control:** Initialize your motor drivers by assigning them to one of the 8 available motor slots, configuring their specific CAN IDs and driver profiles directly in your application code.