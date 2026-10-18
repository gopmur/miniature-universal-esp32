==============================
HTTP Server & Module Subsystem
==============================

The FUMCARE-RP HTTP subsystem provides a modular, object-oriented wrapper around the native ESP-IDF ``httpd`` server. It abstracts away boilerplate code for JSON parsing, asynchronous request handling, WebSocket management, and Cross-Origin Resource Sharing (CORS). It also features an automated build-time pipeline for serving compressed static web assets directly from flash memory.

Architecture Overview
=====================

The web architecture is divided into two primary components:

1. **HttpServer**: The core manager that initializes the ESP-IDF HTTP/HTTPS server (based on Kconfig parameters) and binds the global static asset routes.
2. **HttpModule**: A base class used to group related API endpoints. Modules can be nested (a module can contain sub-modules) to create clean, hierarchical URI routing.

Creating and Registering a Module
=================================

To create a new set of API endpoints, inherit from the ``HttpModule`` class and implement the ``register_direct_uris()`` method. 

.. code-block:: cpp

   #pragma once
   #include "http/module.hpp"

   class HttpExampleModule : public HttpModule {
     private:
     static esp_err_t get_status(httpd_req_t* req);
     void register_direct_uris() override;

     public:
     using HttpModule::HttpModule;
   };

In the source file, bind your handlers to specific routes using the provided registration methods:

.. code-block:: cpp

   #include "http_example_module.hpp"

   esp_err_t HttpExampleModule::get_status(httpd_req_t* req) {
     return send_message_json(req, "Status OK");
   }

   void HttpExampleModule::register_direct_uris() {
     // Registers GET /example/status
     register_uri("/status", HTTP_GET, get_status);
   }

Endpoint Registration API
=========================

The ``HttpModule`` base class provides several protected methods to register endpoints. All registration methods automatically route requests through internal middleware that enforces maximum content lengths and applies global headers (like CORS and Keep-Alive).

Synchronous Endpoints
---------------------

* **Standard Handler:**
  ``register_uri(const char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t* req))``
* **JSON-Auto-Parsing Handler:**
  ``register_uri(const char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json))``
  
  Automatically parses the request body into a ``JsonObject``. If parsing fails, it rejects the request with a 400 Bad Request before hitting your handler.

Asynchronous Endpoints
----------------------

Long-running requests (like OTA updates or hardware scanning) should not block the main HTTP server thread. FUMCARE-RP integrates with ``jaythread`` to dispatch these requests to background threads.

* **Standard Async Handler:**
  ``register_async_uri(const char* uri, httpd_method_t method, ThreadWithArg<httpd_req_t*>* handler)``
* **JSON Async Handler:**
  ``register_async_uri(const char* uri, httpd_method_t method, ThreadWithArg<HttpJsonAsyncHandlerArgs>* handler)``

**Example of an Async JSON Handler:**

.. code-block:: cpp

   class PutAsyncHandler : public ThreadWithArg<HttpJsonAsyncHandlerArgs> {
     private:
     void main(HttpJsonAsyncHandlerArgs* args) override {
       auto req = args->req;
       auto json = args->json;
       // ... perform long running task ...
       HttpModule::send_success_json(req);
       httpd_req_async_handler_complete(req);
     }
   };

   // In register_direct_uris:
   register_async_uri("/process", HTTP_PUT, &put_async_handler_instance);

CORS & Preflight Requests
-------------------------

If your web frontend runs on a different host during development, it will send HTTP ``OPTIONS`` preflight requests. Use the ``_with_option`` suffix when registering routes to automatically generate a compliant ``OPTIONS`` handler.

* ``register_uri_with_option(...)``
* ``register_async_uri_with_option(...)``

WebSocket Endpoints
-------------------

WebSockets are integrated directly into the module system. You must provide a standard handler and a post-handshake callback.

* ``register_ws_uri(const char* uri, esp_err_t (*handler)(httpd_req_t*), esp_err_t (*post_handshake)(httpd_req_t*))``

.. code-block:: cpp

   esp_err_t HttpStreamModule::ws_motor_post_handshake(httpd_req_t* req) {
     auto client_fd = httpd_req_to_sockfd(req);
     ws_task.add_connection(client_fd, WebSocketConnectionType::MOTOR);
     return ESP_OK;
   }

Helper Methods
==============

Inside your handlers, use the inherited static helper methods to format responses easily:

* ``send_json(httpd_req_t* req, Json& json)``
* ``send_message_json(httpd_req_t* req, const char* message, httpd_err_code_t status)``
* ``send_success_json(httpd_req_t* req)``

Static File Serving & Asset Generation
======================================

FUMCARE-RP serves the frontend single-page application (SPA) directly from the ESP32's internal flash memory. To maximize flash efficiency and network speed, the platform compresses all web assets at build time and serves them natively via GZIP.

The Pipeline
------------

1. **Asset Compression:** The python script ``main/scripts/generate_http_assets.py`` recursively scans your compiled web app directory (e.g., ``app/hexa-webapp/dist``).
2. **GZIP Optimization:** It compresses relevant HTML, JS, CSS, and SVG files into ``.gz`` format using maximum compression (level 9) and places them in the ``generated/assets/`` directory.
3. **C++ Code Generation:** The script dynamically writes ``main/inc/http_assets.hpp``. For every compressed file, it generates a C++ macro (``DEFINE_FILE_GET_HANDLER``) that maps the binary blob directly into an HTTP handler.
4. **Binary Embedding:** The ESP-IDF build system uses CMake to embed these ``.gz`` files directly into the firmware image using assembly linking (``_binary_filename_start`` and ``_binary_filename_end``).

How it works at Runtime
-----------------------

When the ``HttpServer`` is started in ``main.cpp``, it calls ``http_server_register_assets()`` (from the generated header). 

When a browser requests a file (e.g., ``/index.html``):
1. The server intercepts the request via the generated handler.
2. It sets the ``Content-Type`` dynamically based on the file extension.
3. It sets the ``Content-Encoding: gzip`` header.
4. It streams the raw binary data directly from flash memory to the network socket via ``httpd_resp_send()``. 

Because the data is already pre-compressed on the host machine, the ESP32 requires zero CPU overhead to serve complex web applications.