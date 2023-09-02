#include <utils/Logger.hpp>
#include "http/Headers.hpp"

using HTTP::Headers;

std::string Headers::FormatKey(const std::string& key) {
  std::string formatted = key;
  Utils::toLowercase(formatted);
  Utils::trim(formatted);
  return formatted;
}

std::string Headers::FormatValue(const std::string& value) {
  std::string formatted = value;
  Utils::trim(formatted);
  return formatted;
}

Headers::Headers() : headers() {}
Headers::~Headers() {}
Headers::Headers(const Headers& other) : headers(other.headers), keys(other.keys) {}
Headers& Headers::operator=(const Headers& other) {
  if (this == &other) return *this;
  this->headers = other.headers;
  this->keys = other.keys;
  return *this;
}

void Headers::clear() {
  this->headers.clear();
  this->keys.clear();
}

bool Headers::append(const std::string& key, const std::string& value) {
  if (this->has(key))
    return false;
  std::string formatted = this->FormatKey(key);
  std::string formattedValue = this->FormatValue(value);
  this->headers.insert(std::make_pair(formatted, formattedValue));
  this->keys.push_back(formatted);
  return true;
}

void Headers::remove(const std::string& key) {
  std::string formatted = this->FormatKey(key);
  if (!this->has(formatted))
    return;
  this->headers.erase(formatted);
  std::vector<std::string>::iterator it = std::find(this->keys.begin(), this->keys.end(), formatted);
  if (it != this->keys.end())
    this->keys.erase(it);
}

bool Headers::has(const std::string& key) const {
  std::string formatted = this->FormatKey(key);
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
    if (!this->headers.count(key)) {
      Logger::error
        << "Headers key " << Logger::param(key) << " not found in headers map"
        << std::newl;
      continue;
    }
    const std::string& value = this->headers.at(key);
    result += HTTP::capitalizeHeader(key) + ": " + value + "\r\n";
  }
  return result;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Headers& headers) {
  os << headers.toString();
  return os;
}