#include <stdlib.h>
#include <cstdint>

#include <arpa/inet.h>
#include "cc.h"
#include "dns/packet/consts.hpp"
#include "dns/packet/packet.hpp"
#include "esp_err.h"
#include "lwip/sockets.h"

#include "helper.hpp"

bool drop_packet(DNSPacket& packet) {
  return packet.get_header().is_response();
}

void set_dns_rcode(DNSPacket& packet, int parse_err) {
  auto header = packet.get_header();
  if (parse_err) {
    header.set_rcode(RCODE_SERVER_FAILURE);
  } else if (header.get_number_of_questions() != 0 ||
             header.get_opcode() == OPCODE_IQUERY ||
             header.get_opcode() == OPCODE_IQUERY) {
    header.set_rcode(RCODE_NOT_IMPLEMENTED);
  } else if (strcmp(packet.get_question().get_name(), "app.local") != 0) {
    header.set_rcode(RCODE_NAME_ERR);
  } else {
    header.set_rcode(RCODE_NO_ERR);
  }
}

void make_dns_answer(DNSPacket& packet) {
  packet.get_header().set_number_of_answers(1);
  packet.get_answer().set_rr_type(RRTYPE_A);
  packet.get_answer().set_rr_class(RRCLASS_IN);
  packet.get_answer().set_ttl(1);
  packet.get_answer().set_rdata("192.168.4.1");
}

void make_dns_response(DNSPacket& packet, int parse_err) {
  packet.get_header().set_response();
  set_dns_rcode(packet, parse_err);
  make_dns_answer(packet);
}

void answer_dns_question() {}

void dns_service_start(in_addr_t iface_address) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in sock_addres = {};
  sock_addres.sin_addr.s_addr = iface_address;
  sock_addres.sin_port = htons(53);
  sock_addres.sin_family = AF_INET;

  ESP_ERROR_CHECK(
      bind(sock, (struct sockaddr*)&sock_addres, sizeof(struct sockaddr_in)));

  constexpr int BUFFER_SIZE = 64;
  static char buf[BUFFER_SIZE];

  DNSPacket packet;

  while (true) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    recvfrom(sock, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client_address,
             &client_address_len);

    int ret = packet.parse(buf, BUFFER_SIZE, nullptr);
    packet.print();
    if (drop_packet(packet)) {
      continue;
    }
    make_dns_response(packet, ret);
    packet.print();
    int answer_size;
    packet.copy(buf, BUFFER_SIZE, &answer_size);
    sendto(sock, buf, answer_size, 0, (struct sockaddr*)&client_address,
           client_address_len);
  }
}