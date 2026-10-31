==============================
HTTP Server & Module Subsystem
==============================

The HTTP subsystem wraps the ESP-IDF ``esp_http_server`` API in two layers:

* ``HttpServer`` starts the HTTP or HTTPS server, registers generated static
  web assets, and asks the root module to register its API routes.
* ``HttpModule`` groups routes into modules. A module can register routes of
  its own and contain child modules, which makes the URL structure follow the
  module tree.

The module registration helpers provide synchronous, JSON-aware, asynchronous,
and WebSocket interfaces. Synchronous HTTP routes pass through common
middleware for response headers and request-size checks. WebSocket routes and
generated static-asset routes are registered separately and do not pass through
that middleware.

Server startup
==============

``HttpServer`` is declared in ``main/inc/http.hpp``. Its public interface is:

.. code-block:: cpp

   HttpServer(HttpModule* root_module);
   void start();
   httpd_handle_t server_instance;

Construct it with a long-lived root module and call ``start()`` once the
application is ready to accept requests. ``start()`` selects the server mode
at compile time:

* With ``CONFIG_HEXA_HTTP_USE_HTTPS`` enabled, it starts the ESP-IDF HTTPS
  server and loads the PEM certificate and private key embedded from
  ``main/certs/ecc_cert.crt`` and ``main/certs/ecc_private.key``.
* Otherwise, it starts the regular HTTP server.

Both modes enable the server's keep-alive support and use the configured
maximum socket and URI-handler counts. After starting the server,
``HttpServer`` registers the generated static assets and then calls
``root_module->register_uris(server_instance)``. Registering the modules
therefore happens as part of server startup; application code normally does
not need to call ``register_uris`` itself.

The public ``server_instance`` is the ESP-IDF server handle and can be used
with ESP-IDF APIs when needed. The ``HttpServer`` API does not currently
provide a stop or restart method.

Module structure and URL construction
=====================================

Derive each route group from ``HttpModule`` and implement its pure virtual
``register_direct_uris()`` method. The base class offers two constructors:

.. code-block:: cpp

   HttpModule(const char* name);
   HttpModule(const char* name, std::vector<HttpModule*> modules);

The first creates a module with no children. The second attaches child
modules. Child pointers are non-owning: keep each module alive for as long as
the server may handle requests. A parent registers its child modules
recursively before registering its own direct routes.

Each registered route is formed from the parent's prefix, the current
module's name, and the URI passed to the registration helper. The root module
starts with the prefix ``/``. For example, a root module named ``api``, a
child named ``system``, and a grandchild named ``wifi`` produce the prefix
``/api/system/wifi``. Registering ``"/scan"`` in that Wi-Fi module produces
``/api/system/wifi/scan``.

The special URI ``"/"`` means the module's base path itself: the final slash
is removed when the route is registered. Thus ``"/"`` in the ``wifi`` module
above registers ``/api/system/wifi``. Use a slash at the start of ordinary
route URIs for clarity. An empty URI is rejected; a non-empty URI without a
leading slash is only logged as a warning, and registration continues, so do
not rely on such paths.

Implementing a module
---------------------

Registration methods and response helpers are ``protected``. They are
available to the derived module and its member functions, but not to callers
of the module object. Route callbacks must have the function-pointer
signatures shown below; instance-member functions are not compatible unless
wrapped by a suitable static callback.

.. code-block:: cpp

   #include "http/module.hpp"

   class HttpExampleModule : public HttpModule {
    public:
     explicit HttpExampleModule() : HttpModule("example") {}

    private:
     static esp_err_t get_status(httpd_req_t* req);
     static esp_err_t put_settings(httpd_req_t* req, JsonObject* body);
     void register_direct_uris() override;
   };

   esp_err_t HttpExampleModule::get_status(httpd_req_t* req) {
     JsonObject response;
     response.set("status", "ready");
     return send_json(req, response);
   }

   esp_err_t HttpExampleModule::put_settings(httpd_req_t* req,
                                              JsonObject* body) {
     // Read and validate fields from body.
     return send_success_json(req);
   }

   void HttpExampleModule::register_direct_uris() {
     register_uri("/status", HTTP_GET, get_status);
     register_uri_with_option("/settings", HTTP_PUT, put_settings);
   }

With the example module placed under a root named ``api``, its routes are
``GET /api/example/status`` and ``PUT /api/example/settings`` (plus the
automatically registered ``OPTIONS /api/example/settings`` route).

``register_direct_uris()`` is called by the base class during
``register_uris()``. It should contain the route declarations; do not call it
from the constructor because the server handle is not assigned until
registration.

Registration interfaces
=======================

The following methods are protected members of ``HttpModule``. In the
signatures below, ``uri`` is the module-relative URI, ``method`` is an
ESP-IDF ``httpd_method_t`` value such as ``HTTP_GET`` or ``HTTP_PUT``, and
``handler`` has the exact callback type shown.

Synchronous handlers
--------------------

**Plain handler**

.. code-block:: cpp

   void register_uri(
       const char* uri,
       httpd_method_t method,
       esp_err_t (*handler)(httpd_req_t* req));

Use this form when the callback reads the request itself or does not need a
JSON request body. The callback runs on the HTTP server task, so it should
finish promptly. It receives the ESP-IDF request pointer and must send the
response itself, typically using one of the response helpers below.

**JSON-parsing handler**

.. code-block:: cpp

   void register_uri(
       const char* uri,
       httpd_method_t method,
       esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json));

The middleware reads and parses the request body as a ``JsonObject`` before
calling the callback. The callback receives a pointer to that parsed object;
use it during the callback and do not retain the pointer. If the body is
larger than the configured maximum, middleware responds with HTTP 413. If
JSON parsing fails, the handler is not called and the response is HTTP 400
with ``{"message":"json parse error"}``.

The synchronous JSON overload explicitly refuses ``HTTP_GET`` and logs a
warning instead of registering the route. Use a body-bearing method such as
``HTTP_POST`` or ``HTTP_PUT`` for JSON input. The plain handler overload can
be registered with any method supported by ESP-IDF.

Asynchronous handlers
---------------------

Use an asynchronous registration method for work that should not occupy the
HTTP server task while it runs, for example a scan or a long-running device
operation. The handler object must derive from the matching
``ThreadWithArg<T>`` specialization and must remain alive while requests can
reach it.

**Plain asynchronous handler**

.. code-block:: cpp

   void register_async_uri(
       const char* uri,
       httpd_method_t method,
       ThreadWithArg<httpd_req_t*>* handler);

The registered thread's override has the signature
``void main(httpd_req_t** req)``. The middleware begins an asynchronous
request and passes its request pointer to the worker. The worker is
responsible for sending the final response and calling
``httpd_req_async_handler_complete(req)`` exactly once when finished.

**JSON asynchronous handler**

.. code-block:: cpp

   void register_async_uri(
       const char* uri,
       httpd_method_t method,
       ThreadWithArg<HttpJsonAsyncHandlerArgs>* handler);

Its worker overrides
``void main(HttpJsonAsyncHandlerArgs* args)``. ``HttpJsonAsyncHandlerArgs``
contains:

.. code-block:: cpp

   struct HttpJsonAsyncHandlerArgs {
     httpd_req_t* req;
     JsonObject json;
   };

The middleware checks the body size and parses JSON on the HTTP server task,
then passes the asynchronous request and a copy of the parsed object to the
worker. This avoids doing long-running work on the server task, but JSON
parsing itself still happens there. As with the plain async handler, the
worker must send its response and complete the asynchronous request exactly
once.

.. code-block:: cpp

   class RefreshHandler : public ThreadWithArg<httpd_req_t*> {
    private:
     void main(httpd_req_t** req) override {
       send_success_json(*req);
       httpd_req_async_handler_complete(*req);
     }
   };

   // Keep this handler object alive for the lifetime of the route.
   static RefreshHandler refresh_handler;

   void HttpExampleModule::register_direct_uris() {
     register_async_uri("/refresh", HTTP_POST, &refresh_handler);
   }

Each ``ThreadWithArg`` instance permits only one active run at a time. If a
second request tries to start the same instance while it is running, the
middleware returns HTTP 500 with ``{"message":"async handler is already
running"}``. Use distinct handler objects for independent operations that
must be able to run concurrently.

The JSON asynchronous registration method accepts a method argument and does
not apply the synchronous JSON overload's explicit ``HTTP_GET`` guard.
Nevertheless, JSON request bodies are intended for body-bearing methods; do
not design a GET route that depends on a request body.

OPTIONS preflight registration
------------------------------

The ``_with_option`` variants register the requested method and an additional
``HTTP_OPTIONS`` route at the same URI. The generated OPTIONS callback sends
an empty response; it does not call the application callback.

.. code-block:: cpp

   void register_uri_with_option(
       const char* uri,
       httpd_method_t method,
       esp_err_t (*handler)(httpd_req_t* req));

   void register_uri_with_option(
       const char* uri,
       httpd_method_t method,
       esp_err_t (*handler)(httpd_req_t* req, JsonObject* req_json));

   void register_async_uri_with_option(
       const char* uri,
       httpd_method_t method,
       ThreadWithArg<httpd_req_t*>* handler);

   void register_async_uri_with_option(
       const char* uri,
       httpd_method_t method,
       ThreadWithArg<HttpJsonAsyncHandlerArgs>* handler);

The same middleware rules apply to the actual route: JSON parsing for JSON
handlers, request-size validation, and configured global headers. The
OPTIONS handler itself uses the plain middleware. Use these methods when a
browser client needs preflight for a route; registering an ordinary route
does not create an OPTIONS route automatically.

WebSocket registration
----------------------

.. code-block:: cpp

   void register_ws_uri(
       const char* uri,
       esp_err_t (*handler)(httpd_req_t* req),
       esp_err_t (*post_handshake_handler)(httpd_req_t* req));

This registers a WebSocket route using ``HTTP_GET`` and marks it as a
WebSocket endpoint in ESP-IDF. The first callback is the URI handler; the
second is the post-handshake callback, invoked after the WebSocket handshake
succeeds. The latter is a suitable place to associate the connected socket
with an application-level stream, for example:

.. code-block:: cpp

   esp_err_t HttpExampleModule::ws_events(httpd_req_t* req) {
     return ESP_OK;
   }

   esp_err_t HttpExampleModule::ws_events_post_handshake(httpd_req_t* req) {
     int client_fd = httpd_req_to_sockfd(req);
     // Add client_fd to the appropriate application connection manager.
     return ESP_OK;
   }

WebSocket registration does not provide an OPTIONS variant and does not pass
through the ``HttpModule`` HTTP middleware. In particular, module-level
request-size checks and configured CORS/keep-alive response headers do not
apply to this route. Configure and manage WebSocket connections in the
application callbacks.

Common middleware and configuration
===================================

All four ordinary HTTP middleware paths (plain, JSON, async, and async JSON)
set configured global response headers, then compare ``req->content_len``
with ``CONFIG_HEXA_HTTP_MAX_REQUEST_LEN``. Oversized requests receive HTTP
413 and a JSON response whose ``message`` states the configured maximum.
For JSON registrations, parse failure receives HTTP 400 before the handler
runs. These middleware checks do not apply to WebSocket or generated
static-asset routes.

The options are configured in ``main/config/Kconfig.http``:

* ``CONFIG_HEXA_HTTP_USE_HTTPS`` selects HTTPS instead of HTTP. It defaults
  to enabled.
* ``CONFIG_HEXA_HTTP_MAX_REQUEST_LEN`` limits the request body length. Its
  Kconfig range is 0 through 1024 bytes; the default is 512.
* ``CONFIG_HEXA_HTTP_ALLOW_CORS`` enables the module middleware's CORS
  response headers. It defaults to disabled. The headers allow origin ``*``,
  methods ``GET, PUT, POST, OPTIONS``, and the ``Content-Type`` request
  header. This is a permissive browser policy, not authentication or
  authorization.
* ``CONFIG_HEXA_HTTP_KEEP_ALIVE`` adds a ``Connection: keep-alive`` response
  header in module middleware and defaults to enabled. Independently, the
  HTTP server itself has keep-alive enabled, and generated asset handlers set
  ``Connection: close``.
* ``CONFIG_HEXA_HTTP_MAX_OPEN_SOCKET`` sets the server's socket limit and
  defaults to 16.
* ``CONFIG_HEXA_HTTP_MAX_URI_HANDLER_COUNT`` sets the route registration
  limit and defaults to 128. Each ``_with_option`` route consumes an
  additional URI-handler slot.

Response and JSON helpers
=========================

These protected static helpers are available from derived ``HttpModule``
classes and their handlers:

.. code-block:: cpp

   static esp_err_t send_json(httpd_req_t* req, Json& json);
   static esp_err_t send_json(
       httpd_req_t* req, Json& json, httpd_err_code_t status);
   static esp_err_t send_message_json(
       httpd_req_t* req, const char* message);
   static esp_err_t send_message_json(
       httpd_req_t* req, const char* message, httpd_err_code_t status);
   static esp_err_t send_success_json(httpd_req_t* req);
   static std::variant<JsonObject, JsonError> parse_json(httpd_req_t* req);

``send_json`` serializes the supplied ``Json`` value, sets the response type
to ``application/json``, and sends it. The status overload sends the value
as an HTTP error response using the supplied ESP-IDF status code.
``send_message_json`` builds an object with a ``message`` property and sends
it, optionally with an error status. ``send_success_json`` is shorthand for
``{"message":"success"}``.

``parse_json`` returns either a parsed ``JsonObject`` or a ``JsonError``.
Most request handlers should use the JSON-aware registration overload and
receive the already-parsed object rather than parsing the same request body
again. The other header helpers are ``set_type_json(req)``,
``allow_cors(req)``, and ``set_keep_alive(req)``; the latter two set response
headers and are also called conditionally by the common middleware.

Static web assets
=================

The web frontend can be embedded in the firmware and served from flash. The
frontend must first be built with the Node.js build system from
``app/hexa-webapp``. For example, using bun:

.. code-block:: bash

   cd app/hexa-webapp
   bun install
   bun run build
   cd ../..

Then run the Python asset generator from the repository root:

.. code-block:: bash

   python main/scripts/generate_http_assets.py app/hexa-webapp/dist generated

The generator recursively compresses frontend files with GZIP level 9,
creates ``generated/inc/http_assets.hpp``, and generates a GET handler and
route for each compressed asset. ESP-IDF embeds the files listed under
``generated/assets/*.gz`` in ``main/CMakeLists.txt``. At startup,
``http_server_register_assets(server_instance)`` registers those routes
before the module API routes.

At request time, each generated handler sets the MIME type inferred from the
original file extension, adds ``Content-Encoding: gzip``, sets
``Connection: close``, and sends the embedded bytes. The browser transparently
decompresses the response. These generated routes do not use
``HttpModule`` registration or its middleware. The generator maps the root
``index.html`` to ``/`` and other files to their corresponding asset paths;
it does not add a general single-page-application fallback for arbitrary
unknown URLs.
