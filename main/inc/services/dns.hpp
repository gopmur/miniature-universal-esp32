#pragma once

#include <sys/types.h>

#include "services/dns/packet.hpp"

class DNSService {
 private:
  in_addr_t iface_address;

  bool drop_packet(DNSPacket& packet);
  void set_dns_rcode(DNSPacket& packet, int parse_err);
  void make_dns_answer(DNSPacket& packet);
  void make_dns_response(DNSPacket& packet, int parse_err);

 public:
  DNSService(const char* iface_address);

  void start();
};
