// #include <cstring>

// #include "cc.h"
// #include "esp_err.h"

// #include "services/dns/packet/header.hpp"

// void DNSHeader::hton() {
//   transaction_id = htons(transaction_id);
//   flags.u16 = htons(flags.u16);
//   number_of_questions = htons(number_of_questions);
//   number_of_answers = htons(number_of_answers);
//   number_of_authority_rrs = htons(number_of_authority_rrs);
//   number_of_additional_rrs = htons(number_of_additional_rrs);
// }

// void DNSHeader::ntoh() {
//   transaction_id = ntohs(transaction_id);
//   flags.u16 = ntohs(flags.u16);
//   number_of_questions = ntohs(number_of_questions);
//   number_of_answers = ntohs(number_of_answers);
//   number_of_authority_rrs = ntohs(number_of_authority_rrs);
//   number_of_additional_rrs = ntohs(number_of_additional_rrs);
// }

// DNSHeader::DNSHeader() {
//   transaction_id = 0;
//   flags.u16 = 0;
//   number_of_questions = 0;
//   number_of_answers = 0;
//   number_of_authority_rrs = 0;
//   number_of_additional_rrs = 0;
// }

// int DNSHeader::copy(char* dest, int size, int* bytes_written) {
//   if (bytes_written)
//     *bytes_written = 0;
//   if (dest == nullptr) {
//     return ESP_ERR_INVALID_ARG;
//   }
//   if (size < DNSHeader::STATIC_SIZE) {
//     return ESP_ERR_NO_MEM;
//   }
//   this->hton();
//   if (bytes_written)
//     *bytes_written = DNSHeader::STATIC_SIZE;
//   memcpy(dest, this, DNSHeader::STATIC_SIZE);
//   this->ntoh();
//   return ESP_OK;
// }

// int DNSHeader::parse(char* src, int size, int* bytes_read) {
//   if (bytes_read)
//     *bytes_read = 0;
//   if (src == nullptr) {
//     return ESP_ERR_INVALID_ARG;
//   }
//   if (size < DNSHeader::STATIC_SIZE) {
//     return ESP_ERR_NO_MEM;
//   }
//   memcpy(this, src, DNSHeader::STATIC_SIZE);
//   *bytes_read = DNSHeader::STATIC_SIZE;
//   this->ntoh();
//   return ESP_OK;
// };

// DNSFlags DNSHeader::get_flags() const {
//   return this->flags;
// }

// uint16_t DNSHeader::get_transaction_id() const {
//   return transaction_id;
// }
// uint16_t DNSHeader::get_number_of_questions() const {
//   return number_of_questions;
// }
// uint16_t DNSHeader::get_number_of_answers() const {
//   return number_of_answers;
// }
// uint16_t DNSHeader::get_number_of_authority_rrs() const {
//   return number_of_authority_rrs;
// }
// uint16_t DNSHeader::get_number_of_additional_rrs() const {
//   return number_of_additional_rrs;
// }

// bool DNSHeader::is_query() const {
//   return flags.b.QR == 0;
// }
// bool DNSHeader::is_response() const {
//   return flags.b.QR == 1;
// }

// Opcode DNSHeader::get_opcode() const {
//   return static_cast<Opcode>(flags.b.OPCODE);
// }
// Rcode DNSHeader::get_rcode() const {
//   return static_cast<Rcode>(flags.b.RCODE);
// }

// void DNSHeader::set_number_of_answers(uint16_t number_of_answers) {
//   this->number_of_answers = number_of_questions;
// }

// void DNSHeader::set_rcode(Rcode rcode) {
//   flags.b.RCODE = rcode;
// }
// void DNSHeader::set_opcode(Opcode opcode) {
//   flags.b.OPCODE = opcode;
// }
// void DNSHeader::set_query() {
//   flags.b.QR = 0;
// }
// void DNSHeader::set_response() {
//   flags.b.QR = 1;
// }

// void DNSHeader::set_aa() {
//   this->flags.b.AA = 1;
// }
// void DNSHeader::clear_aa() {
//   this->flags.b.AA = 0;
// }
// void DNSHeader::set_ra() {
//   this->flags.b.RA = 1;
// }