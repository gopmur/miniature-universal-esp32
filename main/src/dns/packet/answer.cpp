#include <cstring>

#include "cc.h"

#include "dns/packet/header.hpp"
#include "dns/packet/answer.hpp"

void DNSAnswer::hton() {
  name_ref = htons(name_ref);
  type = static_cast<RRType>(htons(type));
  clss = static_cast<RRClass>(htons(clss));
  ttl = htonl(ttl);
  rdlength = htons(rdlength);
  rdata = htonl(rdata);
}

void DNSAnswer::ntoh() {
  name_ref = ntohs(name_ref);
  type = static_cast<RRType>(ntohs(type));
  clss = static_cast<RRClass>(ntohs(clss));
  ttl = ntohl(ttl);
  rdlength = ntohs(rdlength);
  rdata = ntohl(rdata);
}

DNSAnswer::DNSAnswer() {
  // Size fo dns packets are reduced by
  // using references when duplicate names
  // are used. references are started with two 1 bits.
  // For more information please
  // refer to RFC 1035
  name_ref = 0xc000 | sizeof(DNSHeader);
  type = RRType_A;
  clss = RRClass_IN;
  ttl = 0;
  rdlength = 4;
  rdata = 0;
}

int DNSAnswer::copy(void* dest, int size, int* bytes_written) {
  if (bytes_written == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  *bytes_written = 0;
  if (dest == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  if (size < sizeof(DNSAnswer)) {
    return ESP_ERR_NO_MEM;
  }
  *bytes_written = sizeof(DNSAnswer);
  this->hton();
  memcpy(this, dest, sizeof(DNSAnswer));
  this->ntoh();
  return ESP_OK;
}

uint16_t DNSAnswer::get_name_ref() {
  return name_ref;
}

RRType DNSAnswer::get_rr_type() {
  return type;
}

RRClass DNSAnswer::get_rr_class() {
  return clss;
}

uint32_t DNSAnswer::get_ttl() {
  return ttl;
}

uint16_t DNSAnswer::get_rdlength() {
  return rdlength;
}

uint32_t DNSAnswer::get_rdata() {
  return rdata;
}
