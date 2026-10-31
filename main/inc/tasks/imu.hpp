#pragma once

#include "jaythread/thread.hpp"
#include "system_logger.hpp"
#include "custom_drivers/imu.hpp"

class ImuTask : public Thread {
  MAKE_LOGGABLE("imu_task");
  
  public:
  ImuData data;
  Imu* imu;

  private:
  void main();
};