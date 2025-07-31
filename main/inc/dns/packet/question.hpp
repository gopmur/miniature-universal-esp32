#pragma once

#include <cstdint>

constexpr int NAME_MAX_LEN = 255;

class DNSQuestion {
 private:
  char name[NAME_MAX_LEN];
  uint16_t type;
  uint16_t clss;

  void hton();
  void ntoh();

 public:
  DNSQuestion();

  char* get_name();
  uint16_t get_type();
  uint16_t get_class();

  int copy(char* dest, int size, int* bytes_written);
  int parse(char* src, int size, int* bytes_read);
};