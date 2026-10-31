==============================
IMU Sensor Task and Orientation Data
==============================

The inertial measurement unit is handled by ``ImuTask`` in
``main/inc/tasks/imu.hpp``. The task reads data from the onboard ICM-20948
sensor and publishes the latest orientation and acceleration values to the rest of
the system.

Hardware interface
==================

The IMU is connected over I2C. The driver stack exposes a single public object:

.. code-block:: cpp

   class ImuTask : public Thread {
     public:
       ImuData data;
       Imu* imu;
   };

The data object contains the most recent sensor values and is consumed by:

* the logger task
* the WebSocket telemetry broadcaster
* the control logic when attitude feedback is required

Processing model
================

The IMU task continuously polls the sensor and updates the ``ImuData`` object.
The information includes the quaternion-derived orientation sample and the raw
acceleration values. The project uses the DMP-style hardware path and converts the
sensor output into engineering units and Euler angles.

The calculations are organized around:

* quaternion normalization
* roll-pitch-yaw conversion
* acceleration scaling
* timestamped telemetry output

The main orientation fields are exposed as:

* ``gyro.x``
* ``gyro.y``
* ``gyro.z``

These correspond to the three rotational axes that are later serialized into
JSON for the browser or control dashboard.

Telemetry output
================

The IMU data is sent to clients through a WebSocket stream. See the
:doc:`Complete API Reference <api>` for the endpoint details.

The WebSocket task fills a JSON object with:

.. code-block:: json

   {
     "accel": {"x": 0.0, "y": 0.0, "z": 0.0},
     "gyro": {"x": 0.0, "y": 0.0, "z": 0.0},
     "microseconds": 123456789
   }

The browser can then plot the values or use them for balance and gait logic.

Configuration
=============

The IMU task behavior is configured through Kconfig values such as:

* ``CONFIG_HEXA_IMU_SDA_PIN``
* ``CONFIG_HEXA_IMU_SCL_PIN``
* ``CONFIG_HEXA_TASKS_IMU_FREQ_HZ``

These parameters determine how the sensor is initialized and how often the task
updates the data model.

Operational notes
=================

The IMU is not treated as a one-time startup device. It is a continuous telemetry
source. In normal operation, the board reads and republishes the current sensor
state at task frequency, allowing the dashboard to present live motion data.
