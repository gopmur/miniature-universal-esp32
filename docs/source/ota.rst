====================================
Over-The-Air (OTA) Update Subsystem
====================================

The FUMCARE-RP platform features a fully asynchronous, secure Over-The-Air (OTA) update mechanism. This subsystem allows the ESP32 to fetch and install new firmware binaries over a Wi-Fi connection without physical serial access, while simultaneously broadcasting its download progress to connected web clients.

Architecture Overview
=====================

The OTA update process is managed by the **HttpUpdateModule**, which wraps the native **esp_https_ota** and **esp_http_client** libraries. 

To prevent a lengthy firmware download from blocking the main HTTP server thread (which would freeze the web dashboard and other API calls), the update sequence is offloaded to a dedicated background thread using the **ThreadWithArg** abstraction.

Security & Certificate Bundling
-------------------------------
Firmware binaries are downloaded exclusively over HTTPS. The HTTP client is explicitly configured with **esp_crt_bundle_attach**, which validates the distribution server's SSL/TLS certificates against the ESP-IDF default certificate bundle to prevent man-in-the-middle attacks.

The Update Workflow
===================

1. Version Checking
-------------------
When the frontend requests an update check, the **GetCheckHandler** background thread is spawned. It reaches out to the distribution server at **https://<SERVER_ADDRESS>/latest-version**. The server's response is captured and forwarded back to the client so the frontend can compare the remote version against the local core and API versions.

2. Firmware Downloading
-----------------------
If an update is initiated, the **PutHandler** background thread takes over. 
* It sets the global **ota_busy** flag to true.
* It dynamically formats the download URI based on the requested version string (e.g., **https://<SERVER_ADDRESS>/firmware/<VERSION>.bin**).
* It initializes the **esp_https_ota_begin** process.

3. Real-Time Progress Broadcasting
----------------------------------
During the download loop (**esp_https_ota_perform**), the module continuously queries the bytes read versus the total image size. It passes these integers directly to **ws_task.send_to_ota_connections()**. The **WebSocketTask** then packages this into a JSON payload containing **total** and **progress** fields and pushes it to any browser currently monitoring the OTA WebSocket stream.

4. Finalization
---------------
Once the download successfully completes, the OTA handle is finished and a final WebSocket frame is dispatched containing an **ok: true** flag, signaling the frontend to prompt the user to restart the device.

HTTP & WebSocket APIs
=====================

The **HttpUpdateModule** exposes the following endpoints:

* **GET /api/update/check:** Spawns an asynchronous thread to fetch the latest available firmware version from the remote distribution server.
* **PUT /api/update/:** Expects a JSON payload containing the requested **version** string. It triggers the asynchronous download and flashing process.
* **GET /api/update/status:** Returns a JSON object containing a **busy** boolean, indicating if an OTA flash is currently in progress.

The **HttpStreamModule** exposes the WebSocket telemetry:

* **WS /api/stream/ota:** A dedicated WebSocket endpoint that streams the real-time byte progress of the ongoing firmware download.

Kconfig Parameters
==================

The behavior of the OTA subsystem is configured via the ESP-IDF menuconfig under the **OTA** section:

* **Server Address:** Defined by **CONFIG_HEXA_OTA_SERVER_ADDRESS**. This sets the root IP address or domain name (including port) of your firmware distribution server (default: **192.168.4.2:3000**).