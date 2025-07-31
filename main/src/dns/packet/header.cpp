#include "cc.h"
#include "lwip/err.h"

#include "dns/packet/header.hpp"

void DNSHeader::hton() {
  transaction_id = htons(transaction_id);
  flags.u16 = htons(flags.u16);
  number_of_questions = htons(number_of_questions);
  number_of_answers = htons(number_of_answers);
  number_of_authority_rrs = htons(number_of_authority_rrs);
  number_of_additional_rrs = htons(number_of_additional_rrs);
}

void DNSHeader::ntoh() {
  transaction_id = ntohs(transaction_id);
  flags.u16 = ntohs(flags.u16);
  number_of_questions = ntohs(number_of_questions);
  number_of_answers = ntohs(number_of_answers);
  number_of_authority_rrs = ntohs(number_of_authority_rrs);
  number_of_additional_rrs = ntohs(number_of_additional_rrs);
}

DNSHeader::DNSHeader() {
  transaction_id = 0;
  flags.u16 = 0;
  number_of_questions = 0;
  number_of_answers = 0;
  number_of_authority_rrs = 0;
  number_of_additional_rrs = 0;
}

int DNSHeader::copy(void* dest, int size, int* bytes_written) {
  if (bytes_written)
    *bytes_written = 0;
  if (dest == nullptr) {
    return ERR_ARG;
  }
  if (size < sizeof(DNSHeader)) {
    return ERR_BUF;
  }
  this->hton();
  if (bytes_written)
    *bytes_written = sizeof(DNSHeader);
  memcpy(this, dest, sizeof(DNSHeader));
  this->ntoh();
  return ERR_OK;
}

int DNSHeader::parse(void* src, int size, int* bytes_read) {
  if (bytes_read)
    *bytes_read = 0;
  if (src == nullptr) {
    return ERR_ARG;
  }
  if (size < sizeof(DNSHeader)) {
    return ERR_BUF;
  }
  memcpy(this, src, sizeof(DNSHeader));
  *bytes_read = sizeof(DNSHeader);
  this->ntoh();
  return ERR_OK;
};

uint16_t DNSHeader::get_transaction_id() {
  return transaction_id;
}
DNSFlags DNSHeader::get_flags() {
  return flags;
}
uint16_t DNSHeader::get_number_of_questions() {
  return number_of_questions;
}
uint16_t DNSHeader::get_number_of_answers() {
  return number_of_answers;
}
uint16_t DNSHeader::get_number_of_authority_rrs() {
  return number_of_authority_rrs;
}
uint16_t DNSHeader::get_number_of_additional_rrs() {
  return number_of_additional_rrs;
}