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
  std::string formatted = key;
  Utils::toLowercase(formatted);
  this->headers.insert(std::make_pair(formatted, value));
  this->keys.push_back(formatted);
  return true;
}

void Headers::set(const std::string& key, const std::string& value) {
  std::string formatted = key;
  Utils::toLowercase(formatted);
  if (this->headers.count(formatted) == 0)
    this->keys.push_back(formatted);
  this->headers[formatted] = value;
}

void Headers::remove(const std::string& key) {
  std::string formatted = key;
  Utils::toLowercase(formatted);
  this->headers.erase(formatted);
  this->keys.erase(std::remove(this->keys.begin(), this->keys.end(), formatted), this->keys.end());
}

bool Headers::has(const std::string& key) const {
  std::string formatted = key;
  Utils::toLowercase(formatted);
  return this->headers.find(formatted) != this->headers.end();
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
    std::vector<std::string>::const_iterator it = this->keys.begin();
    it != this->keys.end();
    it++) {
    std::string key = *it;
    const std::string& value = this->headers.at(key);
    result += HTTP::capitalizeHeader(key) + ": " + value + "\r\n";
  }
  return result;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Headers& headers) {
  os << headers.toString();
  return os;
}