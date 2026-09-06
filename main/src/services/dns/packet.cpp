#include "services/dns/packet.hpp"
#include "lwip/err.h"

int DNSPacket::parse(char* src, int size, int* bytes_read) {
  if (bytes_read)
    *bytes_read = 0;
  if (src == nullptr) {
    return ERR_ARG;
  }
  int bytes_read_section = 0;
  int src_index = 0;
  int ret = header.parse(&src[src_index], size, &bytes_read_section);
  if (ret)
    return ret;
  src_index += bytes_read_section;
  if (bytes_read)
    *bytes_read += bytes_read_section;
  ret = question.parse(&src[src_index], size - bytes_read_section,
                       &bytes_read_section);
  return ret;
}

int DNSPacket::copy(char* dest, int size, int* bytes_written) {
  if (bytes_written)
    *bytes_written = 0;
  if (dest == nullptr) {
    return ERR_ARG;
  };
  int bytes_written_section = 0;
  int dest_index = 0;
  int ret = header.copy(&dest[dest_index], size, &bytes_written_section);
  *bytes_written += bytes_written_section;
  dest_index += bytes_written_section;
  if (ret) {
    return ret;
  }

  ret = question.copy(&dest[dest_index], size - *bytes_written,
                      &bytes_written_section);
  dest_index += bytes_written_section;
  *bytes_written += bytes_written_section;
  if (ret) {
    return ret;
  }

  ret = answer.copy(&dest[dest_index], size - *bytes_written,
                    &bytes_written_section);
  *bytes_written += bytes_written_section;
  return ret;
};

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

DNSHeader& DNSPacket::get_header() {
  return header;
}
DNSQuestion& DNSPacket::get_question() {
  return question;
}
DNSAnswer& DNSPacket::get_answer() {
  return answer;
}