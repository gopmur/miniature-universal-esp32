#include <stdlib.h>

#include <arpa/inet.h>
#include "cc.h"
#include "dns/packet/packet.hpp"
#include "esp_err.h"
#include "lwip/sockets.h"

// enum DNSParserState {
//   DNS_PARSER_STATE_HEADER,
//   DNS_PARSER_STATE_HOSTNAME,
//   DNS_PARSER_STATE_TYPE_CLASS,
// };

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
//   memcpy(&send_buffer[buffer_index], &packet->question.type,
//   sizeof(uint16_t)); buffer_index += 2; memcpy(&send_buffer[buffer_index],
//   &packet->question.clss, sizeof(uint16_t)); buffer_index += 2;
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
//   memcpy(&send_buffer[buffer_index], &packet->answer.rdata,
//   sizeof(uint32_t)); buffer_index += 4;

//   sendto(sock, send_buffer, buffer_index, 0, (struct
//   sockaddr*)&client_address,
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

void dns_service_start(in_addr_t iface_address) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in sock_addres = {};
  sock_addres.sin_addr.s_addr = iface_address;
  sock_addres.sin_port = htons(53);
  sock_addres.sin_family = AF_INET;

  ESP_ERROR_CHECK(
      bind(sock, (struct sockaddr*)&sock_addres, sizeof(struct sockaddr_in)));

  constexpr int RX_BUFFER_SIZE = 64;
  static uint8_t rx_buf[RX_BUFFER_SIZE];

  DNSPacket packet;

  while (true) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    recvfrom(sock, rx_buf, RX_BUFFER_SIZE, 0, (struct sockaddr*)&client_address,
             &client_address_len);
    packet.parse(reinterpret_cast<char*>(rx_buf), RX_BUFFER_SIZE, nullptr);
    packet.print();
  }
}