=====================================
System Logging and Diagnostic Output
=====================================

The project contains a dedicated logging abstraction in ``main/inc/system_logger.hpp``.
It wraps the ESP-IDF log system and gives every subsystem a stable tag-based log
namespace.

Logging model
=============

The project uses the ``MAKE_LOGGABLE`` macro to inject a private static tag into
classes. Example:

.. code-block:: cpp

   class HttpModule {
     MAKE_LOGGABLE("http_module");
   };

This tag is then used by the project-level log helpers:

* ``LOGV``
* ``LOGD``
* ``LOGI``
* ``LOGW``
* ``LOGE``

The macros route directly into ESP-IDF logging while preserving the source tag.

SystemLogger behavior
=====================

``SystemLogger`` owns the file-backed runtime diagnostics built on top of the
ESP-IDF log stream. It installs a custom log printf handler and sends each log
message to three destinations:

1. the active SD-card log file
2. the WebSocket log stream when connected
3. the serial console

The log file is managed with a mutex to avoid cross-thread corruption during
simultaneous writes. The log file is closed and renamed when the system clock is
available.

Log file lifecycle
==================

The logger starts in a temporary location and later renames the file once the
board successfully synchronizes time. Typical behavior:

* initial log: ``/sd/sys.log``
* later rotation: timestamped name in the same directory

This allows the system to keep a trace of boot-time events and later diagnostic
activity without losing the original session.

WebSocket log stream
===================

A browser client can subscribe to the system log stream through its WebSocket
endpoint. See the :doc:`Complete API Reference <api>` for endpoint details.

This is a live view of the board's logs. It is separate from the CSV telemetry
stream and is oriented toward textual diagnostics.

Operational use
===============

The log utilities are essential for debugging:

* network configuration and Wi-Fi state changes
* OTA progress and failure messages
* motor and control errors
* boot-time faults and runtime exceptions

This is the primary debugging surface when the ESP32 is not attached through a
physical serial console.
