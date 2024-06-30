/**
 * std.hpp
 * Add features that aren't present in cpp98.
*/
#pragma once

#include <sstream>
#include <iostream>
#include <netinet/in.h>

namespace std {
  in_addr_t inet_addr(const char* str);
  std::string inet_ntoa(struct in_addr addr);
}