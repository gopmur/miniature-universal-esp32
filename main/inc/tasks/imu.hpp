#pragma once

#include "jaythread/thread.hpp"

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
  public:
  ImuData data;

  private:
  std::string tag = "imu task";
  void main();
};