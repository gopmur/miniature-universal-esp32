#include "dns/packet/question.hpp"
#include "dns/packet/consts.hpp"

DNSQuestion::DNSQuestion(int name_len) {
  name = name_len == 0 ? nullptr : new char[name_len];
  this->name_len = name_len;
  type = RRType_A;
  clss = RRClass_IN;
}