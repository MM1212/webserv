#include "http/Headers.hpp"

using HTTP::Headers;

Headers::Headers() : headers() {}
Headers::~Headers() {}
Headers::Headers(const Headers& other) : headers(other.headers) {}
Headers& Headers::operator=(const Headers& other) {
  if (this == &other) return *this;
  this->headers = other.headers;
  return *this;
}

void Headers::clear() {
  this->headers.clear();
}

bool Headers::append(const std::string& key, const std::string& value) {
  if (this->has(key))
    return false;
  this->set(key, value);
  return true;
}

void Headers::set(const std::string& key, const std::string& value) {
  this->headers[key] = value;
}

void Headers::remove(const std::string& key) {
  this->headers.erase(key);
}

bool Headers::has(const std::string& key) const {
  return this->headers.find(key) != this->headers.end();
}

const std::string& Headers::get(const std::string& key) const {
  static const std::string empty;
  if (!this->has(key))
    return empty;
  return this->headers.at(key);
}

const std::map<std::string, std::string>& Headers::getAll() const {
  return this->headers;
}

Headers::operator std::string() const {
  return this->toString();
}

std::string Headers::toString() const {
  std::string result;
  for (
    std::map<std::string, std::string>::const_iterator it = this->headers.begin();
    it != this->headers.end();
    it++) {
    result += it->first + ": " + it->second + "\r\n";
  }
  return result;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Headers& headers) {
  os << headers.toString();
  return os;
}