=======================================
Detailed Control Subsystem Architecture
=======================================

The FUMCARE-RP platform delegates all real-time kinematic calculations and hardware dispatching to the **ControlTask**. This dedicated FreeRTOS thread runs an infinite loop with a strict 10-millisecond delay interval, ensuring highly deterministic response times for robotic actuation.

Base Controller & Torque Profiling
==================================

All specific control modes inherit from the base **Controller** class. The most critical shared utility provided by this base class is the **torque_profile()** method. 

To prevent sudden, jerky motor movements that could harm the patient, FUMCARE-RP does not apply square-wave torque. Instead, it generates a smooth, three-phase sigmoid curve based on the current **count_timer** and the **total_time** allocated for the step. 

The time is split into three phases:
* **Phase A (Ramp Up):** $0.15 \times total\_time$.
* **Phase B (Hold):** $0.70 \times total\_time$.
* **Phase C (Ramp Down):** $0.15 \times total\_time$.

During the ramp-up phase, the torque multiplier is calculated as:
$$Multiplier = \frac{1}{1 + e^{-c_1 \times (t - \frac{a}{2})}}$$
Where $c_1 = \frac{10}{a}$.

During the ramp-down phase, the multiplier mirrors this using:
$$Multiplier = \frac{1}{1 + e^{c_2 \times (t - (a + b + \frac{c}{2}))}}$$
Where $c_2 = \frac{10}{c}$.

Detailed Control Modes
======================

1. Zero Controller
------------------
The **ZeroController** acts as the system's absolute fallback. Regardless of incoming IMU or encoder data, its **run()** method strictly populates the output array with **0** for all motors. The **ControlTask** automatically reverts to this mode if the **running** boolean is false or an undefined mode is requested.

2. Manual Controller
--------------------
The **HexaManualController** maps user-defined static torque values directly to the motors. 
* **Input Parameters:** **left.torque** and **right.torque**.
* **Logic:** Applies the exact requested torque continuously without any time limit or velocity checking.

3. Automatic Controller
-----------------------
The **HexaAutomaticController** implements a continuous walking-gait state machine.

* **NVS Namespace:** **hexa_automatic**.
* **NVS Keys & Defaults:** **left_vel_th** (1.4311), **right_vel_th** (1.4311), **angle_th** (1.4311).
* **Logic Flow:** 
  For the right leg to trigger assistance, it must meet three simultaneous conditions:
  1. The right motor velocity exceeds **configs.get_right_velocity_threshold()**.
  2. The delta angle (left position minus right position) exceeds **configs.get_angle_threshold()**.
  3. The opposing (left) motor velocity is less than or equal to 0.
  Once triggered, it applies the sigmoid torque profile scaled by **params.right.torque** for exactly **params.right.timeout** ticks.

4. Semi-Automatic Controller
----------------------------
The **HexaSemiautomaticController** is specifically designed for hemiparetic patients, where only one leg (the **weak_leg**) requires robotic assistance.

* **NVS Namespace:** **hexa_semi_auto**.
* **NVS Keys & Defaults:** **start_angle** (0.2879), **stop_angle** (2.583), **epsilon** (0.0286).
* **Logic Flow (Assuming Right Leg is Weak):**
  1. The controller monitors the angular difference (left minus right).
  2. Assistance prepares to activate if the difference exceeds the **start_assist_angle** AND both the left and right motor velocities drop below the **epsilon** threshold (indicating the patient has planted their healthy foot and stabilized).
  3. Once activated, a timer increments until it reaches **params.right.delay**.
  4. After the delay, the torque is applied. 
  5. The assistance terminates either when the delta angle falls below negative **stop_assist_angle**, or the **params.right.timeout** is exceeded.

5. Smart Controller
-------------------
The **HexaSmartController** utilizes a dynamic fuzzy-logic evaluation matrix.

* **NVS Namespace:** **hexa_smart**.
* **NVS Keys & Defaults:** **vel_th** (0.8).
* **Logic Flow:** 
  The **calculate_output** function takes the delta angle, velocity, and the previous state. To prevent dangerous oscillation, the controller utilizes two crucial timers:
  * **fuzzy_timer:** Must exceed 300 ticks before a new state calculation is permitted.
  * **delay_timer:** A cross-leg lockout. If the right leg activates, the left leg's delay timer is reset, and the left leg is blocked from activating if its delay timer is under 20 ticks. This absolutely prevents the exoskeleton from triggering both legs to swing simultaneously.

HTTP API Reference
==================

The **HttpControlModule** exposes the following strict REST API structure:

State Endpoints
---------------
* **PUT /api/control/start:** Activates the system.
* **PUT /api/control/stop:** Deactivates the system.
* **GET /api/control/params:** Retrieves the master state. The JSON response includes **running**, **mode**, and a deeply nested **controlParams** object containing the properties of every single mode simultaneously.

Mode Selection Endpoints
------------------------
* **PUT /api/control/set-mode/manual**
* **PUT /api/control/set-mode/automatic**
* **PUT /api/control/set-mode/semiautomatic**
* **PUT /api/control/set-mode/smart**

Parameter Tuning Endpoints
--------------------------
These endpoints update the volatile RAM parameters (lost on reboot).

* **PUT /api/control/manual/params:** Requires JSON with **left** and **right** objects, each containing a **torque** float.
* **PUT /api/control/automatic/params:** Requires **left** and **right** objects, each containing **torque** and **timeout** floats.
* **PUT /api/control/semiautomatic/params:** Requires a **weakLeg** string (left or right), plus **left** and **right** objects containing **torque**, **timeout**, and **delay** floats.
* **PUT /api/control/smart/params:** Requires **left** and **right** objects containing **torque** floats.

Configuration Tuning Endpoints
------------------------------
These endpoints write directly to the non-volatile storage (NVS) to permanently save patient calibrations.

* **PUT /api/control/automatic/configs:** Requires **angleThreshold**, plus **left** and **right** objects containing a **velocityThreshold** float.
* **PUT /api/control/semiautomatic/configs:** Requires **startAssistAngle**, **stopAssistAngle**, and **epsilon** floats.
* **PUT /api/control/smart/configs:** Requires a **velocityThreshold** float.

Real-Time Telemetry
-------------------
* **WS /api/control/manual/torque:** Expects JSON frames containing **leftTorque** and/or **rightTorque** to instantly update the manual controller.