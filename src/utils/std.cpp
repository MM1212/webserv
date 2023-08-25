#include "utils/std.hpp"
#include <utils/misc.hpp>

// in_addr_t inet_addr(const char* str);
//   std::string inet_ntoa(in_addr_t addr);

// convert an IPv4 address from string to in_addr_t
in_addr_t std::inet_addr(const char* str) {
  in_addr_t result = 0;
  int shift = 24;
  int num = 0;
  while (*str != '\0') {
    if (*str == '.') {
      result |= num << shift;
      shift -= 8;
      num = 0;
    } else {
      num *= 10;
      num += *str - '0';
    }
    str++;
  }
  result |= num << shift;
  return result;
}

// convert an IPv4 address from in_addr_t to string
std::string std::inet_ntoa(struct in_addr s_addr) {
  in_addr_t addr = s_addr.s_addr;
  std::string result;
  result.reserve(15);
  for (int i = 0; i < 4; i++) {
    result += Utils::toString((addr >> (i * 8)) & 0xFF);
    if (i < 3)
      result += ".";
  }
  return result;
}