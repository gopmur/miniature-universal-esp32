// #include <cstring>

// #include "cc.h"
// #include "lwip/sockets.h"

// #include "services/dns/packet/answer.hpp"
// #include "services/dns/packet/consts.hpp"
// #include "services/dns/packet/header.hpp"

// void DNSAnswer::hton() {
//   name_ref = htons(name_ref);
//   type = static_cast<RRType>(htons(type));
//   clss = static_cast<RRClass>(htons(clss));
//   ttl = htonl(ttl);
//   rdlength = htons(rdlength);
// }

// void DNSAnswer::ntoh() {
//   name_ref = ntohs(name_ref);
//   type = static_cast<RRType>(ntohs(type));
//   clss = static_cast<RRClass>(ntohs(clss));
//   ttl = ntohl(ttl);
//   rdlength = ntohs(rdlength);
//   rdata = ntohl(rdata);
// }

// DNSAnswer::DNSAnswer() {
//   // Size fo dns packets are reduced by
//   // using references when duplicate names
//   // are used. references are started with two 1 bits.
//   // For more information please
//   // refer to RFC 1035
//   name_ref = 0xc000 | DNSHeader::STATIC_SIZE;
//   type = RRTYPE_A;
//   clss = RRCLASS_IN;
//   ttl = 0;
//   rdlength = 4;
//   rdata = 0;
// }

// int DNSAnswer::copy(char* dest, int size, int* bytes_written) {
//   if (bytes_written == nullptr) {
//     return ESP_ERR_INVALID_ARG;
//   }
//   *bytes_written = 0;
//   if (dest == nullptr) {
//     return ESP_ERR_INVALID_ARG;
//   }
//   if (size < DNSAnswer::STATIC_SIZE) {
//     return ESP_ERR_NO_MEM;
//   }
//   *bytes_written = DNSAnswer::STATIC_SIZE;
//   int dest_index = 0;
//   this->hton();
//   memcpy(&dest[dest_index], &name_ref, sizeof(uint16_t));
//   dest_index += sizeof(uint16_t);
//   memcpy(&dest[dest_index], &type, sizeof(RRType));
//   dest_index += sizeof(RRType);
//   memcpy(&dest[dest_index], &clss, sizeof(RRClass));
//   dest_index += sizeof(RRClass);
//   memcpy(&dest[dest_index], &ttl, sizeof(uint32_t));
//   dest_index += sizeof(uint32_t);
//   memcpy(&dest[dest_index], &rdlength, sizeof(uint16_t));
//   dest_index += sizeof(uint16_t);
//   memcpy(&dest[dest_index], &rdata, sizeof(uint32_t));
//   this->ntoh();
//   return ESP_OK;
// }

// uint16_t DNSAnswer::get_name_ref() {
//   return name_ref;
// }

// RRType DNSAnswer::get_rr_type() {
//   return type;
// }

// RRClass DNSAnswer::get_rr_class() {
//   return clss;
// }

// uint32_t DNSAnswer::get_ttl() {
//   return ttl;
// }

// uint16_t DNSAnswer::get_rdlength() {
//   return rdlength;
// }

// uint32_t DNSAnswer::get_rdata() {
//   return rdata;
// }

// void DNSAnswer::set_rr_type(RRType rr_type) {
//   this->type = rr_type;
// }
// void DNSAnswer::set_rr_class(RRClass rr_class) {
//   this->clss = rr_class;
// }
// void DNSAnswer::set_ttl(uint32_t ttl) {
//   this->ttl = ttl;
// }
// void DNSAnswer::set_rdata(const char* address) {
//   inet_pton(AF_INET, address, &this->rdata);
// }