/**
 * Headers.hpp
 * HTTP Headers class.
 * Stores a key-value pair of HTTP headers.
 * All keys are formatted to be trimmed & lowercased internally.
 * All values are formatted to be trimmed.
*/
#pragma once

#include <string>
#include <map>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <algorithm>

#include <sstream>
#include <iomanip>

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
    void remove(const std::string& key);
    bool has(const std::string& key) const;

    template <typename T>
    T get(const std::string& key) const {
      if constexpr (std::is_same_v<T, std::string>)
        return this->_getString(key);
      std::string formatted = this->FormatKey(key);
      T value;
      if (!this->has(formatted))
        throw std::runtime_error("Key " + key + " not found");
      std::stringstream ss(this->headers.at(formatted));
      ss >> value;
      return value;
    }

  private:
    std::string _getString(const std::string& key) const {
      std::string formatted = this->FormatKey(key);
      if (!this->has(formatted))
        throw std::runtime_error("Key " + key + " not found");
      return this->headers.at(formatted);
    }
  public:

    const std::map<std::string, std::string>& getAll() const;

    template <typename T>
    void set(const std::string& key, const T& value) {
      if constexpr (std::is_same_v<T, bool>)
        return this->_setBool(key, value);
      else if constexpr (std::is_same_v<T, std::string>)
        return this->_setString(key, value);
      std::stringstream ss;
      ss << value;
      this->set(key, ss.str());
    }

  private:
    void _setBool(const std::string& key, const bool& value) {
      std::stringstream ss;
      ss << std::boolalpha << value;
      this->set(key, ss.str());
    }

    void _setString(const std::string& key, const std::string& value) {
      std::string formatted = this->FormatKey(key);
      std::string formattedValue = this->FormatValue(value);
      if (this->headers.count(formatted) == 0)
        this->keys.push_back(formatted);
      this->headers[formatted] = formattedValue;
    }
  public:
    operator std::string() const;
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Headers& headers);
  private:
    std::map<std::string, std::string> headers;
    std::vector<std::string> keys;
  };

}