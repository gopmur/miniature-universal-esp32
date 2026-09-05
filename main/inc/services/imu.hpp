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

class ImuThread : public Thread {
  public:
  static constexpr const char* tag = "ImuThread";
  ImuData data;

  private:
  void main();
};