===============================
Logger Task and Telemetry Recording
===============================

``LoggerTask`` is a simple but important data-capture worker. It is not the main
real-time controller; instead, it records system telemetry to the SD card in a
CSV format while remaining separate from the control loop.

Task model
===========

The logger class is defined in ``main/inc/tasks/logger.hpp`` and exposes:

* ``start_new_log()``
* ``stop_log()``

The task keeps a private flag named ``is_logging`` and waits for notifications
before entering its active cycle.

The implementation uses a very small state machine:

1. wait for notification
2. open ``/sd/my_log.csv`` in write mode
3. write the header
4. keep recording while ``is_logging`` is true
5. close the file when stopped
6. wait for the next notification

File format
===========

The logger writes a CSV table containing columns for:

* each motor position
* each motor velocity
* IMU gyro values: X, Y, Z

The generated header is created by ``write_header()``:

.. code-block:: text

   motor_0_position,motor_1_position,...,motor_0_velocity,...,imu_gyro_x,imu_gyro_y,imu_gyro_z

The row values are produced by ``write_data()``.

Recording loop
==============

When a log starts, the task opens ``/sd/my_log.csv`` and writes the CSV header
once. While logging is active, it writes a telemetry row at the configured
logger task frequency. Each row contains the motor positions and velocities,
followed by the current IMU gyro values. When logging stops, the task closes the
file.

Logging can be started and stopped through the web API. See the
:doc:`Complete API Reference <api>` for the endpoint details.

Operational responsibility
=========================

This subsystem is oriented toward offline analysis and diagnostics rather than
live control. Its purpose is to capture the device state for later inspection,
so it lives outside the critical control loop and avoids interfering with the
motor update path.
