// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include <arpa/inet.h>
// #include "cc.h"
// #include "esp_err.h"

// #include "socket.h"

// #include "dns.hpp"
// #include "helper.h"






// void dns_header_byte_inverse(DNSPacket* packet) {
//   packet->header.flags.u16 = hton(packet->header.flags.u16);
//   packet->header.number_of_additional_rrs =
//       hton(packet->header.number_of_additional_rrs);
//   packet->header.number_of_answers = hton(packet->header.number_of_answers);
//   packet->header.number_of_authority_rrs =
//       hton(packet->header.number_of_authority_rrs);
//   packet->header.number_of_questions = hton(packet->header.number_of_questions);
//   packet->header.transaction_id = hton(packet->header.transaction_id);
// }

// void dns_question_byte_inverse(DNSPacket* packet) {
//   packet->question.type = hton(packet->question.type);
//   packet->question.clss = hton(packet->question.clss);
// }

// void dns_answer_byte_inverse(DNSPacket* packet) {
//   packet->answer.clss = static_cast<RRClass>(hton(packet->answer.clss));
//   packet->answer.type = static_cast<RRType>(hton(packet->answer.type));
//   packet->answer.rdlength = hton(packet->answer.rdlength);
//   packet->answer.ttl = htonl(packet->answer.ttl);
// }

// void print_dns_packet(DNSPacket* packet) {
//   printf("transaction id: 0x%x\n", packet->header.transaction_id);
//   printf("flags:          0x%x\n", packet->header.flags.u16);
//   printf("questions:      %d\n", packet->header.number_of_questions);
//   printf("answers:        %d\n", packet->header.number_of_answers);
//   printf("authority RRs:  %d\n", packet->header.number_of_authority_rrs);
//   printf("additional RRs: %d\n", packet->header.number_of_additional_rrs);
//   printf("-------------------------------------\n");
//   printf("name:           %s\n", packet->question.name);
//   printf("type:           0x%x\n", packet->question.type);
//   printf("class:          0x%x\n", packet->question.clss);
//   printf("======================================\n");
//   printf("\n");
// }

// void dns_packet_received_callback(int sock,
//                                   struct sockaddr_in client_address,
//                                   socklen_t client_address_len,
//                                   DNSPacket* packet) {
//   if (packet->header.flags.b.QR == 1 ||
//       packet->header.number_of_questions == 0 ||
//       strcmp(packet->question.name, "zephyr.local")) {
//     return;
//   }

//   print_dns_packet(packet);

//   packet->header.flags.b.QR = 1;
//   packet->header.number_of_answers = 1;
//   // packet->header.flags.b.RA = 0;
//   packet->answer.name = new char[2];
//   packet->answer.name[0] = 0xc0;
//   packet->answer.name[1] = 0x0c;
//   packet->answer.clss = RRClass_IN;
//   packet->answer.type = RRType_A;
//   packet->answer.ttl = 1;
//   packet->answer.rdlength = 4;
//   packet->answer.rdata = inet_addr("192.168.4.1");

//   dns_header_byte_inverse(packet);
//   dns_question_byte_inverse(packet);
//   dns_answer_byte_inverse(packet);

//   static uint8_t send_buffer[64];

//   int buffer_index = 0;
//   memcpy(&send_buffer[buffer_index], &packet->header, sizeof(DNSHeader));
//   buffer_index += sizeof(DNSHeader);
//   int label_len_index = buffer_index;
//   buffer_index++;
//   int label_len = 0;
//   int name_index = 0;
//   char c;
//   while ((c = packet->question.name[name_index++])) {
//     if (c == '.') {
//       send_buffer[label_len_index] = label_len;
//       label_len = 0;
//       label_len_index = buffer_index++;
//       continue;
//     }
//     send_buffer[buffer_index] = c;
//     buffer_index++;
//     label_len++;
//   }
//   send_buffer[label_len_index] = label_len;
//   send_buffer[buffer_index] = '\0';
//   buffer_index++;
//   memcpy(&send_buffer[buffer_index], &packet->question.type, sizeof(uint16_t));
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], &packet->question.clss, sizeof(uint16_t));
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], packet->answer.name, 2);
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], &packet->answer.type, sizeof(uint16_t));
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], &packet->answer.clss, sizeof(uint16_t));
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], &packet->answer.ttl, sizeof(uint32_t));
//   buffer_index += 4;
//   memcpy(&send_buffer[buffer_index], &packet->answer.rdlength,
//          sizeof(uint16_t));
//   buffer_index += 2;
//   memcpy(&send_buffer[buffer_index], &packet->answer.rdata, sizeof(uint32_t));
//   buffer_index += 4;

//   sendto(sock, send_buffer, buffer_index, 0, (struct sockaddr*)&client_address,
//          client_address_len);

//   dns_header_byte_inverse(packet);
//   dns_question_byte_inverse(packet);
//   dns_answer_byte_inverse(packet);
// }

// void free_dns_packet(DNSPacket* packet) {
//   if (packet->question.name) {
//     free(packet->question.name);
//   }
//   if (packet->answer.name) {
//     free(packet->answer.name);
//   }
// }

// void dns_service_start(in_addr_t iface_address) {
//   const int buffer_size = 64;
//   int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

//   struct sockaddr_in sock_addres = {};
//   sock_addres.sin_addr.s_addr = iface_address;
//   sock_addres.sin_port = htons(53);
//   sock_addres.sin_family = AF_INET;

//   ESP_ERROR_CHECK(
//       bind(sock, (struct sockaddr*)&sock_addres, sizeof(struct sockaddr_in)));

//   static uint8_t buffer[64];
//   int i = 0;

//   DNSParserState dns_parser_state = DNS_PARSER_STATE_HEADER;
//   while (true) {
//     struct sockaddr_in client_address;
//     socklen_t client_address_len = sizeof(struct sockaddr_in);
//     int bytes_received =
//         recvfrom(sock, buffer, buffer_size, 0,
//                  (struct sockaddr*)&client_address, &client_address_len);

//     i = 0;
//     dns_parser_state = DNS_PARSER_STATE_HEADER;
//     while (i < bytes_received) {
//       if (dns_parser_state == DNS_PARSER_STATE_HEADER) {
//         if (i + sizeof(DNSHeader) > bytes_received) {
//           break;
//         }
//         memcpy(&packet.header, &buffer[i], sizeof(DNSHeader));
//         i += sizeof(DNSHeader);
//         dns_parser_state = DNS_PARSER_STATE_HOSTNAME;
//         dns_header_byte_inverse(&packet);
//       } else if (dns_parser_state == DNS_PARSER_STATE_HOSTNAME) {
//         int label_len = buffer[i];
//         if (i + label_len > bytes_received) {
//           break;
//         }
//         if (label_len == 0 && packet.question.name != NULL) {
//           packet.question.name[packet.question.name_len - 1] = '\0';
//           // here name_len will represent the string len not array len
//           packet.question.name_len--;
//           dns_parser_state = DNS_PARSER_STATE_TYPE_clss;
//           i++;
//           continue;
//         }
//         i++;
//         // len here represent the array len not string len
//         int name_len = packet.question.name_len;
//         int new_name_len = name_len + label_len + 1;
//         packet.question.name =
//             static_cast<char*>(realloc(packet.question.name, new_name_len));
//         memcpy(&packet.question.name[name_len], &buffer[i], label_len);
//         packet.question.name[new_name_len - 1] = '.';
//         packet.question.name_len = new_name_len;
//         i += label_len;
//       } else if (dns_parser_state == DNS_PARSER_STATE_TYPE_CLASS) {
//         if (i + 2 * sizeof(uint16_t) > bytes_received) {
//           break;
//         }
//         memcpy(&packet.question.type, &buffer[i], sizeof(uint16_t));
//         i += sizeof(uint16_t);
//         memcpy(&packet.question.clss, &buffer[i], sizeof(uint16_t));
//         i += sizeof(uint16_t);
//         dns_question_byte_inverse(&packet);
//         dns_answer_byte_inverse(&packet);
//         dns_packet_received_callback(sock, client_address, client_address_len,
//                                      &packet);
//         free_dns_packet(&packet);
//       }
//     }
//   }
// }