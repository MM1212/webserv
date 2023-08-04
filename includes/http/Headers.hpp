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
      T value;
      if (!this->has(key))
        throw std::runtime_error("Key " + key + " not found");
      std::stringstream ss(this->headers.at(key));
      ss >> value;
      return value;
    }

    template <>
    const std::string& get(const std::string& key) const {
      static const std::string empty;
      if (!this->has(key))
        throw std::runtime_error("Key " + key + " not found");
      return this->headers.at(key);
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