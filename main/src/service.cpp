#include "service.hpp"
#include "thread.hpp"

std::vector<_AbstractService*> _AbstractService::service_list;

_AbstractService::_AbstractService(int stack_size, int priority, const char* name)
    : _AbstractThread(name, priority, stack_size) {
  this->name = static_cast<char*>(malloc(strlen(name)));
  strcpy(this->name, name);
}

void _AbstractService::_main(_AbstractService* self) {
  self->main();
  self->suspend();
}
