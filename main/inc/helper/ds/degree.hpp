#pragma once

#include <cmath>
class Degree {
  private:
  double _rad;
  Degree();

  public:
  inline static Degree from_rad(double rad);
  inline static Degree from_deg(double deg);

  void rad(double rad);
  void deg(double deg);
  double rad();
  double deg();
};

inline Degree Degree::from_deg(double deg) {
  Degree d;
  d.deg(deg);
  return d;
}

inline Degree Degree::from_rad(double rad) {
  Degree d;
  d.rad(rad);
  return d;
}

inline void Degree::rad(double rad) {
  _rad = rad;
}

inline void Degree::deg(double deg) {
  _rad = M_PI * deg / 180;
}

inline double Degree::deg() {
  return _rad * 180 * M_1_PI;
}

inline double Degree::rad() {
  return _rad;
}