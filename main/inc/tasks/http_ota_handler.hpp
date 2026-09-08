// #pragma once

// #include "esp_http_server.h"
// #include "ipc/queue.hpp"
// #include "service.hpp"

// class HttpOtaHandlerService : public Service<4096> {
//   public:
//   Queue<httpd_req_t*, 1> queue;
//   HttpOtaHandlerService(int priority);
//   void main();
// };