======================
IMU Subsystem & Task
======================

The FUMCARE-RP platform integrates a dedicated **ImuTask** to continuously monitor and process spatial orientation and acceleration data. This subsystem interfaces natively with the onboard ICM-20948 9-axis Inertial Measurement Unit over an I2C bus.

Architecture Overview
=====================

To relieve the ESP32's primary processing cores from complex sensor fusion mathematics, the IMU task leverages the ICM-20948's internal Digital Motion Processor (DMP). 

The architecture handles:
* **Hardware Initialization:** Verifying I2C connectivity and bootstrapping the DMP.
* **Continuous Polling:** A FreeRTOS thread that continuously drains the sensor's FIFO buffer.
* **Kinematic Conversion:** Translating raw DMP quaternions into usable Euler angles (Roll, Pitch, Yaw) for robotic balance and gait analysis.

Hardware Initialization & DMP Configuration
===========================================

When the **ImuTask** boots, it performs a strict initialization sequence:

1. **I2C Binding:** Connects to the sensor using the configured SDA and SCL pins at a 400kHz clock speed.
2. **Device Verification:** Repeatedly queries the **WHO_AM_I** register until the correct device ID is confirmed, preventing the system from proceeding if the sensor is disconnected.
3. **Sensor Scaling:** Sets the accelerometer full-scale range to 2G (**GPM_2**) and the gyroscope to 250 degrees per second (**DPS_250**).
4. **DMP Booting:** Enables the internal Digital Motion Processor, specifically requesting the **Quat9** (9-axis sensor fusion) and raw accelerometer data streams.

Data Processing & Math
======================

The task executes an infinite loop that reads frames from the DMP FIFO buffer. 

Quaternion to Euler Conversion
------------------------------
The DMP provides orientation data as quaternions. The **ImuTask** extracts these (Q1, Q2, Q3) and dynamically calculates Q0 to ensure the quaternion vector is perfectly normalized. It then performs the complex trigonometric conversions to yield standard Euler angles in degrees:

* **Roll:** Calculated using **atan2** based on Q0, Q1, Q2, and Q3.
* **Pitch:** Calculated using **asin** with built-in clamping (between -1.0 and 1.0) to prevent NaN errors during vertical singularities (gimbal lock).
* **Yaw:** Calculated using **atan2** to determine the heading.

These processed degrees are stored in the globally accessible **data.gyro.x**, **data.gyro.y**, and **data.gyro.z** structures.

Accelerometer Scaling
---------------------
Simultaneously, raw accelerometer readings (X, Y, Z) are extracted and divided by a scaling factor of **8192.0f** to convert them into standard G-force units.

Real-Time Telemetry
===================

The processed IMU data is seamlessly integrated into the WebSocket broadcasting subsystem.

When a client connects to the **WS /api/stream/imu** endpoint, the **WebSocketTask** calls the **fill_imu_data_json()** method. This method packages the latest data into a JSON payload:

* **accel:** Contains **x**, **y**, and **z** acceleration values.
* **gyro:** Contains **x**, **y**, and **z** (roll, pitch, yaw) orientation values in degrees.
* **microseconds:** A highly accurate hardware timestamp appended to every frame.

Kconfig Parameters
==================

The IMU hardware and RTOS task properties can be customized via the ESP-IDF menuconfig:

* **SDA pin:** The I2C Serial Data pin, defined by **CONFIG_HEXA_IMU_SDA_PIN** (default: 21).
* **SCL pin:** The I2C Serial Clock pin, defined by **CONFIG_HEXA_IMU_SCL_PIN** (default: 22).
* **Priority:** The FreeRTOS task priority (default: 2).
* **Stack Size:** Memory allocated for the IMU processing thread (default: 4096 bytes).
* **Frequency (Hz):** Dictates the polling rate of the DMP FIFO buffer, defined by **CONFIG_HEXA_TASKS_IMU_FREQ_HZ** (default: 200 Hz). The thread intelligently adjusts its sleep time based on whether more data is already available in the FIFO.