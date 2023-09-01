#include "utils/std.hpp"
#include <utils/misc.hpp>

// in_addr_t inet_addr(const char* str);
//   std::string inet_ntoa(in_addr_t addr);

// convert an IPv4 address from string to in_addr_t
// replicates the behavior of inet_addr from <arpa/inet.h>
in_addr_t std::inet_addr(const char* str) {
  in_addr_t result = 0;
  int i = 0;
  int offset = 0;
  int octet = 0;
  while (str[i] != '\0') {
    if (str[i] == '.') {
      if (octet > 255)
        return INADDR_NONE;
      result |= octet << offset;
      offset += 8;
      octet = 0;
    } else if (str[i] >= '0' && str[i] <= '9') {
      octet = octet * 10 + (str[i] - '0');
    } else {
      return INADDR_NONE;
    }
    i++;
  }
  if (octet > 255)
    return INADDR_NONE;
  result |= octet << offset;
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