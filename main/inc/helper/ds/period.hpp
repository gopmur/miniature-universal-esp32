#pragma once

class Period {
  private:
  double _ns;
  Period();

  public:
  inline static Period from_ns(double ns);
  inline static Period from_us(double us);
  inline static Period from_ms(double ms);
  inline static Period from_s(double s);
  inline static Period from_hz(double hz);
  inline static Period from_khz(double khz);
  inline static Period from_mhz(double mhz);
  inline static Period from_ghz(double ghz);

  inline double ns() const;
  inline double us() const;
  inline double ms() const;
  inline double s() const;
  inline double hz() const;
  inline double khz() const;
  inline double mhz() const;
  inline double ghz() const;

  inline void ns(double ns);
  inline void us(double us);
  inline void ms(double ms);
  inline void s(double s);
  inline void hz(double hz);
  inline void khz(double khz);
  inline void mhz(double mhz);
  inline void ghz(double ghz);
};

inline Period Period::from_ns(double ns) {
  Period p;
  p.ns(ns);
  return p;
}

inline Period Period::from_us(double us) {
  Period p;
  p.us(us);
  return p;
}

inline Period Period::from_ms(double ms) {
  Period p;
  p.ms(ms);
  return p;
}

inline Period Period::from_s(double s) {
  Period p;
  p.s(s);
  return p;
}

inline Period Period::from_hz(double hz) {
  Period p;
  p.hz(hz);
  return p;
}

inline Period Period::from_khz(double khz) {
  Period p;
  p.khz(khz);
  return p;
}

inline Period Period::from_mhz(double mhz) {
  Period p;
  p.mhz(mhz);
  return p;
}

inline Period Period::from_ghz(double ghz) {
  Period p;
  p.ghz(ghz);
  return p;
}

inline void Period::ns(double ns) {
  _ns = ns;
}

inline void Period::us(double us) {
  _ns = us * 1'000;
}

inline void Period::ms(double ms) {
  _ns = ms * 1'000'000;
}

inline void Period::s(double s) {
  _ns = s * 1'000'000'000;
}

inline void Period::hz(double hz) {
  _ns = 1 / hz * 1'000'000'000;
}

inline void Period::khz(double khz) {
  _ns = 1 / khz * 1'000'000;
}

inline void Period::mhz(double mhz) {
  _ns = 1 / mhz * 1'000;
}

inline void Period::ghz(double ghz) {
  _ns = 1 / ghz;
}

inline double Period::ns() const {
  return _ns;
}

inline double Period::us() const {
  return _ns / 1'000;
}

inline double Period::ms() const {
  return _ns / 1'000'000;
}

inline double Period::s() const {
  return _ns / 1'000'000'000;
}

inline double Period::hz() const {
  return 1 / _ns / 1'000'000'000;
}

inline double Period::khz() const {
  return 1 / _ns / 1'000'000;
}

inline double Period::mhz() const {
  return 1 / _ns / 1'000;
}

inline double Period::ghz() const {
  return 1 / _ns;
}
