#pragma once

#include "dns/packet/answer.hpp"
#include "dns/packet/header.hpp"
#include "dns/packet/question.hpp"

struct DNSPacket {
 private:
  DNSHeader header;
  DNSQuestion question;
  DNSAnswer answer;

 public:
  void print();

  int parse(char* buf, int size, int* bytes_read);
  int copy(char* dest, int size, int* bytes_written);

  DNSHeader& get_header();
  DNSQuestion& get_question();
  DNSAnswer& get_answer();
};