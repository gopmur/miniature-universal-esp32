#pragma once

#include <cstdint>

enum RRType : uint16_t {
  RRType_A = 1,
  RRType_NS,
  RRType_MD,
  RRType_MF,
  RRType_CNAME,
  RRType_SOA,
  RRType_MB,
  RRType_MG,
  RRType_MR,
  RRType_NULL,
  RRType_WKS,
  RRType_PTR,
  RRType_HINFO,
  RRType_MINFO,
  RRType_MX,
  RRType_TXT,
};

enum RRClass : uint16_t {
  RRClass_IN = 1,
  RRClass_CS,
  RRClass_CH,
  RRClass_HS,
};