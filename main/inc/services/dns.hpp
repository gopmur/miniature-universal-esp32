#pragma once

#include <sys/types.h>

#include "config.hpp"
#include "service.hpp"
#include "services/dns/packet.hpp"

class DnsService
    : public AbstractService<DnsService, config::service::dns::stack_size> {
  private:
  in_addr_t iface_address;

  bool drop_packet(DNSPacket& packet);
  void set_dns_rcode(DNSPacket& packet, int parse_err);
  void make_dns_answer(DNSPacket& packet);
  void make_dns_response(DNSPacket& packet, int parse_err);
  public:
  void main();
  DnsService(int priority, const char* iface_address);
};
