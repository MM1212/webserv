#include "HTTP.hpp"

using HTTP::Headers;

Headers::Headers() : headers() {}
Headers::~Headers() {}
Headers::Headers(const Headers& other) : headers(other.headers) {}
Headers& Headers::operator=(const Headers& other) {
  if (this == &other) return *this;
  headers = other.headers;
  return *this;
}

void Headers::clear() {
  headers.clear();
}

bool Headers::append(const std::string& key, const std::string& value) {
  if (this->has(key))
    return false;
  this->set(key, value);
  return true;
}

void Headers::set(const std::string& key, const std::string& value) {
  headers[key] = value;
}

bool Headers::has(const std::string& key) const {
  return headers.find(key) != headers.end();
}

const std::string& Headers::get(const std::string& key) const {
  static const std::string empty;
  if (!this->has(key))
    return empty;
  return headers.at(key);
}

Headers::operator std::string() {
  return this->toString();
}

std::string Headers::toString() const {
  std::string result;
  for (
    std::map<std::string, std::string>::const_iterator it = headers.begin();
    it != headers.end();
    it++) {
    result += it->first + ": " + it->second + "\r\n";
  }
  return result;
}