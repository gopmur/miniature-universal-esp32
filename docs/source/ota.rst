=================================
Over-the-Air Firmware Update Flow
=================================

The OTA subsystem is implemented by ``HttpUpdateModule`` and is designed to
update the firmware without a physical serial connection. It uses HTTPS to fetch
new firmware and then reprograms the ESP32 using ESP-IDF OTA support.

The key classes are:

* ``HttpUpdateModule``
* ``GetCheckHandler``
* ``PutHandler``
* ``ota_busy`` flag

Design goals
=============

The OTA path is asynchronous by design. It is intentionally separated from the
main HTTP server thread to avoid blocking the rest of the web stack while a
large firmware image is being downloaded.

The implementation uses:

* ``esp_http_client`` for version checking
* ``esp_https_ota_begin()`` and ``esp_https_ota_perform()`` for flashing
* ``esp_crt_bundle_attach`` for certificate validation
* a background thread via ``ThreadWithArg`` to avoid blocking web requests

The OTA process is not intended to run inline on the HTTP task, because the main
HTTP server must stay responsive for the dashboard and control endpoints.

The update API checks for a release and starts firmware installation
asynchronously. See the :doc:`Complete API Reference <api>` for its endpoints
and request details. The check fetches version metadata from the firmware server
configured by ``CONFIG_HEXA_OTA_SERVER_ADDRESS``.

The HTTP client makes a GET request to:

.. code-block:: text

   https://<SERVER_ADDRESS>/latest-version

The server response is returned directly to the client so that the front-end can
compare local and remote versions.

The asynchronous handler then:

1. sets ``ota_busy`` to true
2. builds the firmware URL using the requested version
3. starts the HTTPS OTA session
4. streams progress updates to the OTA WebSocket
5. finalizes the new firmware image after success

WebSocket progress stream
=========================

The broadcaster sends progress increments as the OTA image is downloaded and
finished. This lets the user interface present firmware installation progress
without polling. See the :doc:`Complete API Reference <api>` for the stream
endpoint.

Security model
==============

The update flow uses HTTPS and verifies the certificate chain through the ESP-IDF
certificate bundle. This reduces the risk of a malicious or misconfigured upstream
server. The OTA handlers also verify that a remote response was valid before
accepting the firmware image.

Operational safety
==================

The implementation deliberately keeps the OTA update process isolated and stateful.
The ``ota_busy`` flag prevents concurrent update attempts while one installation is
running. A second request can therefore be rejected or serialized by the logic,
depending on the implementation state.
