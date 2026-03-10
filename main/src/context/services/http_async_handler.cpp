#include "services/http_async_handler.hpp"
#include "config.hpp"

HttpAsyncHandlerService http_async_handler_service(
    config::service::http_async_handler::priority);