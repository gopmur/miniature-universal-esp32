#pragma once

#include "jaythread/thread.hpp"
#include "system_logger.hpp"

struct ImuVector3D {
  float x;
  float y;
  float z;
};

struct ImuData {
  ImuVector3D accel;
  ImuVector3D gyro;
  float temp;
};

class ImuTask : public Thread {
  MAKE_LOGGABLE("imu_task");
  
  public:
  ImuData data;

  private:
  void main();
};