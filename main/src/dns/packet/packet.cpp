#include "dns/packet/packet.hpp"
#include "lwip/err.h"

int DNSPacket::parse(char* src, int size, int* bytes_read) {
  if (bytes_read)
    *bytes_read = 0;
  if (src == nullptr) {
    return ERR_ARG;
  }
  if (size < sizeof(DNSHeader)) {
    return ERR_BUF;
  };
  int bytes_read_part = 0;
  int src_index = 0;
  int ret = header.parse(&src[src_index], size, &bytes_read_part);
  if (ret)
    return ret;
  src_index += bytes_read_part;
  ret = question.parse(&src[src_index], size - bytes_read_part, &bytes_read_part);
  return ret;
}

void DNSPacket::print() {
  printf("transaction id: 0x%x\n", this->header.get_transaction_id());
  printf("flags:          0x%x\n", this->header.get_flags().u16);
  printf("questions:      %d\n", this->header.get_number_of_questions());
  printf("answers:        %d\n", this->header.get_number_of_answers());
  printf("authority RRs:  %d\n", this->header.get_number_of_authority_rrs());
  printf("additional RRs: %d\n", this->header.get_number_of_additional_rrs());
  printf("-------------------------------------\n");
  printf("name:           %s\n", this->question.get_name());
  printf("type:           0x%x\n", this->question.get_type());
  printf("class:          0x%x\n", this->question.get_class());
  printf("======================================\n");
  printf("\n");
}