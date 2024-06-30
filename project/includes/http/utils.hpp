#pragma once

#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utils/misc.hpp>

namespace HTTP {
  static const std::string specialFieldTokens
    = "!#$%&'*+-.^_`|~;=0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const std::string specialUriTokens
    = "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  bool hasFieldToken(char c);
  bool hasFieldToken(const std::string& str);

  bool hasDisallowedUriToken(char c);
  bool hasDisallowedUriToken(const std::string& str);

  std::string& capitalizeHeader(std::string& str);
}