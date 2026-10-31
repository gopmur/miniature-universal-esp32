#pragma once

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

class Imu {
  protected:
  ImuData data;
  Imu();

  public:
  virtual void init() = 0;
  virtual bool poll() = 0;
  ImuData get_data();
};