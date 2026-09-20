#pragma once

#include "helper/ds/static_string.hpp"

StaticString<17> get_bssid_string(uint8_t bssid[6]);
char get_hex_char(uint8_t byte);
StaticString<2> get_hex(uint8_t byte);