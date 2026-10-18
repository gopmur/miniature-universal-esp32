==================================
System Logger & Telemetry Tracking
==================================

The FUMCARE-RP platform utilizes a dual-purpose logging architecture. It features a core **SystemLogger** for capturing diagnostic messages across the entire firmware, and a dedicated **LoggerTask** for recording high-speed, structured kinematic telemetry to the SD card.

System Diagnostics Logger
=========================

The **SystemLogger** class provides a robust, thread-safe wrapper around the native ESP-IDF logging facility. 

Class Tagging
-------------
To easily identify the source of log messages, the subsystem provides the **MAKE_LOGGABLE(log_tag)** macro. Placing this macro inside a class definition automatically injects a private, static **tag** variable into the class. The system also provides convenient overrides for standard logging macros (**LOGV**, **LOGD**, **LOGI**, **LOGW**, **LOGE**) that automatically utilize this tag.

Tri-Output Routing
------------------
During boot, **SystemLogger::init()** overrides the default ESP-IDF log handler by passing a custom **log_vprintf()** function to **esp_log_set_vprintf()**. This custom function intercepts every log message and routes it to three destinations simultaneously:

1. **SD Card Storage:** It acquires the **log_file_mutex**, writes the formatted string to the active log file, forces an **fsync()**, and flushes the buffer to ensure no data is lost during a crash.
2. **WebSocket Broadcast:** It checks if any clients are connected to the **SYS_LOG** WebSocket stream via **ws_task.has_connection()**. If active, it dynamically allocates a buffer, formats the string, and dispatches it to the **ws_task.sys_log_queue** for wireless browser debugging.
3. **Serial Console:** It passes the arguments to the standard **vprintf()** function for output over the UART cable.

Log Rotation & Timestamping
---------------------------
When the system boots, it begins logging to a temporary file named **/sd/sys.log**. Once the system successfully connects to Wi-Fi and synchronizes the real-time clock via SNTP, the **update_log_file_name()** function is triggered. 

This function acquires the file mutex, closes the temporary log, and renames it using a precise timestamp format (**YYYY-MM-DD-HH-MM-SS.log**). It then seamlessly reopens the file in append mode so logging can continue uninterrupted.

Telemetry Data Logger
=====================

Separate from diagnostic text logs, the **LoggerTask** operates as an independent FreeRTOS thread dedicated to capturing quantitative robotic data.

* **Activation:** It remains suspended, waiting for a signal via **Sync::wait_for_notification_and_clear()**.
* **Execution:** When started, it opens **/sd/my_log.csv** in write mode and immediately writes a CSV header. The header defines columns for every motor's position, every motor's velocity, and the IMU gyroscope's X, Y, and Z axes.
* **Data Loop:** While the **is_logging** boolean is true, it writes the live feedback values from the **motor_task** and **imu_task**, then sleeps for exactly 10 milliseconds before sampling again.
* **Termination:** Once stopped, it closes the CSV file and awaits the next notification.

HTTP & WebSocket APIs
=====================

The **HttpLogModule** exposes endpoints to control telemetry sessions and view live system diagnostics over the network.

* **GET /api/log/session/start:** Triggers the **start_new_log()** method on the **LoggerTask**, beginning the 100Hz CSV data collection.
* **GET /api/log/session/stop:** Triggers the **stop_log()** method, halting CSV data collection and safely closing the file.
* **WS /api/log/system:** Upgrades the connection to a WebSocket and registers the client to receive real-time, asynchronous diagnostic logs directly from the **log_vprintf()** interceptor.