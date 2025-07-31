#pragma once

#include <cstdint>

union DNSFlags {
  struct {
    uint16_t RCODE : 4;
    uint16_t CD : 1;
    uint16_t AD : 1;
    uint16_t Z : 1;
    uint16_t RA : 1;
    uint16_t RD : 1;
    uint16_t TC : 1;
    uint16_t AA : 1;
    uint16_t OPCODE : 4;
    uint16_t QR : 1;
  } b;
  uint16_t u16;
};

class DNSHeader {
 private:
  uint16_t transaction_id;
  DNSFlags flags;
  uint16_t number_of_questions;
  uint16_t number_of_answers;
  uint16_t number_of_authority_rrs;
  uint16_t number_of_additional_rrs;

  void hton();
  void ntoh();

 public:
  DNSHeader();

  int copy(void* dest, int size, int* bytes_written);
  int parse(void* src, int size, int* bytes_read);

  uint16_t get_transaction_id();
  DNSFlags get_flags();
  uint16_t get_number_of_questions();
  uint16_t get_number_of_answers();
  uint16_t get_number_of_authority_rrs();
  uint16_t get_number_of_additional_rrs();
};