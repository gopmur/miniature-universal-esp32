#include "cc.h"

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