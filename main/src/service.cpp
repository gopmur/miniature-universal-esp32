#include "service.hpp"
#include "thread.hpp"

std::vector<ServiceThread*> ServiceThread::service_list;

ServiceThread::ServiceThread(int stack_size, int priority, const char* name)
    : Thread(name, priority, stack_size) {
  this->name = static_cast<char*>(malloc(strlen(name)));
  strcpy(this->name, name);
}

void ServiceThread::_main(ServiceThread* self) {
  self->main();
  self->suspend();
}
