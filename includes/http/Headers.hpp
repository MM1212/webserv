#pragma once

#include <string>
#include <map>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <algorithm>

#include <utils/misc.hpp>
#include <http/utils.hpp>

namespace HTTP {
  class Headers {
  public:
    static std::string FormatKey(const std::string& key);
    static std::string FormatValue(const std::string& value);

    Headers();
    ~Headers();
    Headers(const Headers& other);
    Headers& operator=(const Headers& other);
    void clear();
    bool append(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    void remove(const std::string& key);
    bool has(const std::string& key) const;

    template <typename T>
    T get(const std::string& key) const {
      std::string formatted = this->FormatKey(key);
      T value;
      if (!this->has(formatted))
        throw std::runtime_error("Key " + key + " not found");
      std::stringstream ss(this->headers.at(formatted));
      ss >> value;
      return value;
    }

    template <>
    const std::string& get(const std::string& key) const {
      std::string formatted = this->FormatKey(key);
      if (!this->has(formatted))
        throw std::runtime_error("Key " + key + " not found");
      return this->headers.at(formatted);
    }

    const std::map<std::string, std::string>& getAll() const;

    operator std::string() const;
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Headers& headers);
  private:
    std::map<std::string, std::string> headers;
    std::vector<std::string> keys;
  };

}