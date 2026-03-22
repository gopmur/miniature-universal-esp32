#include "service.hpp"
#include "thread.hpp"

ServiceThread::ServiceThread(int stack_size, int priority, const char* name)
    : Thread(name, priority, stack_size) {}

void ServiceThread::_main(ServiceThread* self) {
  self->main();
  self->suspend();
}
