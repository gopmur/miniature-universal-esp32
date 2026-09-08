#include "tasks/control.hpp"
#include "jaythread/sync.hpp"

void ControlTask::main() {
  while (true) {
    Sync::sleep(1000);
  }
}