#pragma once

#include <cstdint>

#include "dns/packet/consts.hpp"

class DNSAnswer {
 private:
  uint16_t name_ref;
  RRType type;
  RRClass clss;
  uint32_t ttl;
  uint16_t rdlength;
  uint32_t rdata;

  void hton();
  void ntoh();

 public:
  DNSAnswer();

  int copy(void* dest, int size, int* bytes_written);

  uint16_t get_name_ref();
  RRType get_rr_type();
  RRClass get_rr_class();
  uint32_t get_ttl();
  uint16_t get_rdlength();
  uint32_t get_rdata();
};