// #pragma once

// #include <vector>
// #include "config.hpp"
// #include "service.hpp"
// #include "thread.hpp"
// #include "jaythread/ipc/mutex.hpp"

// class MonitorService : public Service<config::service::monitor::stack_size> {
//   private:
//   std::vector<ThreadWrapper> task_list;
//   Mutex task_list_mutex;
//   void update_task_list();

//   public:
//   void main();
//   std::vector<ThreadWrapper> get_task_list();
//   MonitorService(int priority);
// };