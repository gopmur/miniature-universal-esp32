#include "cc.h"
#include "dns/packet/header.hpp"
#include "lwip/err.h"

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
  // Byte order of network may differ from
  // byte order of host.
  this->hton();
}

int DNSAnswer::copy(void* dest, int size, int* bytes_written) {
  *bytes_written = 0;
  if (dest == nullptr) {
    return ERR_ARG;
  }
  if (size < sizeof(DNSAnswer)) {
    return ERR_BUF;
  }
  *bytes_written = sizeof(DNSAnswer);
  this->hton();
  memcpy(this, dest, sizeof(DNSAnswer));
  this->ntoh();
  return ERR_OK;
}

uint16_t DNSAnswer::get_name_ref() {
  return htons(name_ref);
}

RRType DNSAnswer::get_rr_type() {
  return static_cast<RRType>(htons(type));
}

RRClass DNSAnswer::get_rr_class() {
  return static_cast<RRClass>(htons(clss));
}

uint32_t DNSAnswer::get_ttl() {
  return htonl(ttl);
}

uint16_t DNSAnswer::get_rdlength() {
  return htons(rdlength);
}

uint32_t DNSAnswer::get_rdata() {
  return htonl(rdata);
}
