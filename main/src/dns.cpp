#include <stdlib.h>

#include <arpa/inet.h>
#include "cc.h"
#include "dns/packet/packet.hpp"
#include "esp_err.h"
#include "lwip/sockets.h"

#include "helper.hpp"

void dns_service_start(in_addr_t iface_address) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  struct sockaddr_in sock_addres = {};
  sock_addres.sin_addr.s_addr = iface_address;
  sock_addres.sin_port = htons(53);
  sock_addres.sin_family = AF_INET;

  ESP_ERROR_CHECK(
      bind(sock, (struct sockaddr*)&sock_addres, sizeof(struct sockaddr_in)));

  constexpr int RX_BUFFER_SIZE = 30;
  static uint8_t rx_buf[RX_BUFFER_SIZE];

  DNSPacket packet;

  while (true) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(struct sockaddr_in);
    recvfrom(sock, rx_buf, RX_BUFFER_SIZE, 0, (struct sockaddr*)&client_address,
             &client_address_len);
    ESP_CONTINUE_ON_ERROR(
        packet.parse(reinterpret_cast<char*>(rx_buf), RX_BUFFER_SIZE, nullptr));
    packet.print();
  }
}