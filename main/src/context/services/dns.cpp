#include "services/dns.hpp"
#include "config.hpp"

DnsService dns_service(config::service::dns::priority, "192.168.4.1");