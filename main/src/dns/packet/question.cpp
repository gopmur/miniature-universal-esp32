#include "dns/packet/question.hpp"
#include "dns/packet/consts.hpp"
#include "lwip/err.h"

DNSQuestion::DNSQuestion() {
  name[0] = '\0';
  type = RRType_A;
  clss = RRClass_IN;
}

void DNSQuestion::hton() {
  type = htons(type);
  clss = htons(clss);
}

void DNSQuestion::ntoh() {
  type = ntohs(type);
  clss = ntohs(clss);
  char label_len;
  int name_index = 0;
  while ((label_len = name[name_index])) {
    name[name_index] = '.';
    name_index += label_len + 1;
  }
}

int DNSQuestion::copy(char* dest, int size, int* bytes_written) {
  // if (bytes_written)
  //   *bytes_written = 0;
  // if (dest == nullptr) {
  //   return ERR_ARG;
  // }
  // int static_question_size = sizeof(DNSQuestion) - sizeof(char*) -
  // sizeof(int); int dynamic_question_size = strlen(); int question_size =
  // static_question_size + dynamic_question_size; if (size < question_size) {
  //   return ERR_BUF;
  // }
  return 0;
}

int DNSQuestion::parse(char* src, int size, int* bytes_read) {
  if (bytes_read)
    *bytes_read = 0;
  if (src == nullptr) {
    return ERR_ARG;
  }
  int src_index = 0;
  int static_question_size = sizeof(DNSQuestion) - NAME_MAX_LEN;
  if (size < static_question_size) {
    return ERR_BUF;
  }
  char label_len;
  while ((label_len = src[src_index])) {
    if (src_index + label_len + 1 >= size) {
      return ERR_BUF;
    }
    memcpy(&this->name[src_index], &src[src_index], label_len + 1);
    if (bytes_read)
      *bytes_read += label_len + 1;
    src_index += label_len + 1;
  }
  name[src_index] = '\0';
  if (bytes_read)
    (*bytes_read)++;
  src_index++;
  if (src_index + static_question_size - 1 >= size) {
    return ERR_BUF;
  }
  memcpy(&this->type, &src[src_index], sizeof(uint16_t));
  if (bytes_read)
    *bytes_read += 2;
  src_index += 2;
  memcpy(&this->clss, &src[src_index], sizeof(uint16_t));
  if (bytes_read)
    *bytes_read += 2;
  this->ntoh();
  return ERR_OK;
}

char* DNSQuestion::get_name() {
  return &name[1];
}

uint16_t DNSQuestion::get_type() {
  return type;
}

uint16_t DNSQuestion::get_class() {
  return clss;
}