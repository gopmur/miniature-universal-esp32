#include <arpa/inet.h>
#include <stdlib.h>
#include "cc.h"

#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "lwip/sockets.h"

#include "services/dns/packet.hpp"
#include "services/dns/packet/consts.hpp"

#include "services/dns.hpp"

DNSService::DNSService(const char* iface_address) {
  inet_pton(AF_INET, iface_address, &this->iface_address);
}

bool DNSService::drop_packet(DNSPacket& packet) {
  return packet.get_header().is_response();
}

void DNSService::set_dns_rcode(DNSPacket& packet, int parse_err) {
  auto& header = packet.get_header();
  auto& question = packet.get_question();
  if (parse_err) {
    header.set_rcode(RCODE_SERVER_FAILURE);
  } else if (header.get_number_of_questions() != 1 ||
             header.get_opcode() == OPCODE_IQUERY ||
             header.get_opcode() == OPCODE_STATUS ||
             question.get_type() != RRTYPE_A) {
    header.set_rcode(RCODE_NOT_IMPLEMENTED);
  } else if (strcmp(packet.get_question().get_name(), "app.lan") != 0) {
    header.set_rcode(RCODE_NAME_ERR);
  } else {
    header.set_rcode(RCODE_NO_ERR);
    packet.print();
  }
}

void DNSService::make_dns_answer(DNSPacket& packet) {
  packet.get_header().set_number_of_answers(1);
  packet.get_answer().set_rr_type(RRTYPE_A);
  packet.get_answer().set_rr_class(RRCLASS_IN);
  packet.get_answer().set_ttl(1);
  packet.get_answer().set_rdata("192.168.4.1");
}

void DNSService::make_dns_response(DNSPacket& packet, int parse_err) {
  packet.get_header().set_response();
  set_dns_rcode(packet, parse_err);
  if (packet.get_header().get_rcode() == RCODE_NO_ERR) {
    make_dns_answer(packet);
  }
}

[[noreturn]] void DNSService::provider(DNSService* service) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in sock_addres = {};
  sock_addres.sin_addr.s_addr = service->iface_address;
  sock_addres.sin_port = htons(53);
  sock_addres.sin_family = AF_INET;

  ESP_ERROR_CHECK(
      bind(sock, (struct sockaddr*)&sock_addres, sizeof(struct sockaddr_in)));

  constexpr int BUFFER_SIZE = 64;
  static char buf[BUFFER_SIZE];

  static DNSPacket packet;

  while (true) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    recvfrom(sock, buf, BUFFER_SIZE, 0, (struct sockaddr*)&client_address,
             &client_address_len);

    int ret = packet.parse(buf, BUFFER_SIZE, nullptr);
    if (service->drop_packet(packet)) {
      continue;
    }
    service->make_dns_response(packet, ret);
    int answer_size;
    packet.copy(buf, BUFFER_SIZE, &answer_size);
    sendto(sock, buf, answer_size, 0, (struct sockaddr*)&client_address,
           client_address_len);
  }
};

void DNSService::start() {
  xTaskCreate(reinterpret_cast<void (*)(void*)>(DNSService::provider),
              "DNS service", 4096, this, 2, nullptr);
}