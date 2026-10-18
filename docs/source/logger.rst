===========================
Logger Task & CSV Telemetry
===========================

The **LoggerTask** is a dedicated FreeRTOS thread responsible for recording high-speed, quantitative kinematic data directly to the ESP32's SD card. It operates independently of the real-time control loops, ensuring that file I/O operations do not block critical robotic movements.

Architecture Overview
=====================

The logger relies on cross-task communication to gather its data. It directly accesses the global **motor_task** and **imu_task** instances to sample their latest state.

* **Thread Suspension:** To avoid wasting CPU cycles, the task remains blocked via **Sync::wait_for_notification_and_clear()** until a logging session is explicitly requested.
* **Sampling Rate:** When active, the task utilizes **Sync::sleep(10)** between iterations, yielding a 100 Hz recording frequency.

CSV Data Format
===============

Data is formatted as comma-separated values and written to **/sd/my_log.csv**. 

The **write_header()** function establishes the following columns:

* **Motor Positions:** Dynamically generates a **motor_X_position** column for every motor defined by the total motor count.
* **Motor Velocities:** Dynamically generates a **motor_X_velocity** column for every motor.
* **IMU Gyroscope:** Records the roll, pitch, and yaw under **imu_gyro_x**, **imu_gyro_y**, and **imu_gyro_z**.

The **write_data()** function extracts the corresponding values for these columns:

* Queries **motor_task->get_position(i)** for each motor.
* Queries **motor_task->get_velocity(i)** for each motor.
* Reads **imu_task->data.gyro** for the X, Y, and Z axes.

Task Lifecycle & API Control
============================

The logging session is controlled via the **HttpLogModule**, which exposes the following endpoints to the web frontend:

* **GET /api/log/session/start:** Calls **logger_task->start_new_log()**. This sets the **is_logging** flag to true and calls **notify()** to wake the blocked task. The task opens the file in write mode (**"w"**) and begins the recording loop.
* **GET /api/log/session/stop:** Calls **logger_task->stop_log()**. This sets the **is_logging** flag to false, allowing the task to exit its recording loop, safely close the file via **fclose()**, and return to a suspended state awaiting the next notification.

Implementation Note
===================

When reviewing the **LoggerTask::main()** loop, there appears to be a minor typo in the current implementation. Inside the active **while (is_logging)** loop, the task currently calls **write_header()** repeatedly instead of **write_data()**. You may want to swap that function call so your CSV populates with the actual telemetry values rather than repeating the column names on every row!