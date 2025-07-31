#pragma once

#include "dns/packet/answer.hpp"
#include "dns/packet/header.hpp"
#include "dns/packet/question.hpp"

struct DNSPacket {
  DNSHeader header;
  DNSQuestion question;
  DNSAnswer answer;
  DNSPacket(int name_len);
};