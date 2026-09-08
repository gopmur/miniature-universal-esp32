#pragma once

#include <cstdint>

#include "tasks/dns/packet/consts.hpp"

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
  static constexpr int STATIC_SIZE =
      4 * sizeof(uint16_t) + 2 * sizeof(uint32_t);
  DNSAnswer();

  int copy(char* dest, int size, int* bytes_written);

  uint16_t get_name_ref();
  RRType get_rr_type();
  RRClass get_rr_class();
  uint32_t get_ttl();
  uint16_t get_rdlength();
  uint32_t get_rdata();

  void set_rr_type(RRType rr_type);
  void set_rr_class(RRClass rr_class);
  void set_ttl(uint32_t ttl);
  void set_rdata(const char* address);
};