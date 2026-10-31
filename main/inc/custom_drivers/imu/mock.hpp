#include "custom_drivers/imu.hpp"

class MockImu : public Imu {
  private:
  float t = 0;
  static constexpr float t_step = 0.1;

  public:
  void init();
  bool poll();
};