// #include "services/http/helper.hpp"

// char get_hex_char(uint8_t byte) {
//   byte &= 0xf;
//   if (byte >= 0xa) {
//     return 'a' + (byte - 0xa);
//   } else {
//     return '0' + byte;
//   }
// }

// StaticString<2> get_hex(uint8_t byte) {
//   StaticString<2> hex;
//   auto h = byte >> 4;
//   auto l = byte & 0xf;
//   hex.push(get_hex_char(h));
//   hex.push(get_hex_char(l));
//   return hex;
// }

// StaticString<17> get_bssid_string(uint8_t bssid[6]) {
//   StaticString<17> out;
//   for (int i = 0; i < 6; i++) {
//     uint8_t byte = bssid[i];
//     auto byte_str = get_hex(byte);
//     out.push(byte_str[0]);
//     out.push(byte_str[1]);
//     if (i != 5) {
//       out.push(':');
//     }
//   }
//   return out;
// }