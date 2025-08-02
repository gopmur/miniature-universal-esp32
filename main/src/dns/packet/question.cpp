#include "cc.h"

#include "esp_err.h"

#include <cstring>
#include "dns/packet/consts.hpp"
#include "dns/packet/question.hpp"

DNSQuestion::DNSQuestion() {
  name[0] = '\0';
  type = RRTYPE_A;
  clss = RRCLASS_IN;
}

void DNSQuestion::hton() {
  type = htons(type);
  clss = htons(clss);
  char c = 0;
  char label_len = 0;
  char label_len_index = 0;
  int name_index = 0;
  while ((c = name[name_index])) {
    if (c == '.') {
      name[label_len_index] = label_len;
      label_len = 0;
      label_len_index = name_index;
      name_index++;
      continue;
    }
    name_index++;
    label_len++;
  }
  name[label_len_index] = label_len;
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
  if (bytes_written)
    *bytes_written = 0;
  if (dest == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  int dest_index = 0;

  int dynamic_question_size = strlen(this->name) + 1;
  int question_size = DNSQuestion::STATIC_SIZE + dynamic_question_size;
  if (size < question_size) {
    return ESP_ERR_NO_MEM;
  }
  this->hton();
  strcpy(&dest[dest_index], this->name);
  dest_index += dynamic_question_size;
  memcpy(&dest[dest_index], &this->type, sizeof(uint16_t));
  dest_index += sizeof(uint16_t);
  memcpy(&dest[dest_index], &this->clss, sizeof(uint16_t));
  this->ntoh();
  if (bytes_written)
    *bytes_written = question_size;
  this->ntoh();
  return 0;
}

int DNSQuestion::parse(char* src, int size, int* bytes_read) {
  if (bytes_read)
    *bytes_read = 0;
  if (src == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  int src_index = 0;
  if (size < DNSQuestion::STATIC_SIZE) {
    return ESP_ERR_NO_MEM;
  }
  char label_len;
  while ((label_len = src[src_index])) {
    if (src_index + label_len + 1 >= size) {
      return ESP_ERR_NO_MEM;
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
  if (src_index + DNSQuestion::STATIC_SIZE - 1 >= size) {
    return ESP_ERR_NO_MEM;
  }
  memcpy(&this->type, &src[src_index], sizeof(uint16_t));
  if (bytes_read)
    *bytes_read += 2;
  src_index += 2;
  memcpy(&this->clss, &src[src_index], sizeof(uint16_t));
  if (bytes_read)
    *bytes_read += 2;
  this->ntoh();
  return ESP_OK;
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