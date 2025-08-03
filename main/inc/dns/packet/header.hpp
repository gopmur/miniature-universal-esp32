#pragma once

#include <cstdint>
#include "dns/packet/consts.hpp"

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
  static constexpr int STATIC_SIZE = sizeof(DNSFlags) + sizeof(uint16_t) * 5;

  DNSHeader();

  int copy(char* dest, int size, int* bytes_written);
  int parse(char* src, int size, int* bytes_read);

  uint16_t get_transaction_id() const;
  DNSFlags get_flags() const;
  uint16_t get_number_of_questions() const;
  uint16_t get_number_of_answers() const;
  uint16_t get_number_of_authority_rrs() const;
  uint16_t get_number_of_additional_rrs() const;
  bool is_query() const;
  bool is_response() const;
  Opcode get_opcode() const;
  Rcode get_rcode() const;

  void set_number_of_answers(uint16_t number_of_questions);
  void set_rcode(Rcode rcode);
  void set_opcode(Opcode opcode);
  void set_query();
  void set_response();
  void set_aa();
  void set_ra();
  void clear_aa();
};