#include "http/utils.hpp"

bool HTTP::hasFieldToken(char c) {
  return HTTP::specialFieldTokens.find(c) == std::string::npos;
}

bool HTTP::hasFieldToken(const std::string& str) {
  return str.find_first_not_of(HTTP::specialFieldTokens) != std::string::npos;
}

bool HTTP::hasDisallowedUriToken(char c) {
  return HTTP::specialUriTokens.find(c) == std::string::npos;
}

bool HTTP::hasDisallowedUriToken(const std::string& str) {
  return str.find_first_not_of(HTTP::specialUriTokens) != std::string::npos;
}

std::string& HTTP::capitalizeHeader(std::string& str) {
  std::vector<std::string> parts = Utils::split(str, "-");
  str.clear();
  for (
    std::vector<std::string>::iterator it = parts.begin();
    it != parts.end();
    ++it
    ) {
    std::string& part = *it;
    Utils::capitalize(part);
    str += part;
    if (it + 1 != parts.end())
      str += "-";
  }
  return str;
}