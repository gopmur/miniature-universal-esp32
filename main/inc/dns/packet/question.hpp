#pragma once

#include <cstdint>

struct DNSQuestion {
  char* name;
  int name_len;
  uint16_t type;
  uint16_t clss;
  DNSQuestion(int name_len);
};