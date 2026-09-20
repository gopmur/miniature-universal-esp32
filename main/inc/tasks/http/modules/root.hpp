#pragma once

#include "tasks/http/module.hpp"

class HttpRootModule : public HttpModule {
  public:
  void register_direct_uris();
  using HttpModule::HttpModule;
};