/**
 * misc.hpp
 * Various utility functions.
*/
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdint.h>
#include <ctime>
#include <sys/time.h>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <exception>
#include <shared.hpp>

namespace Utils
{
  std::vector<std::string> split(const std::string& str, std::string delim);
  std::string& capitalize(std::string& str);

  template <typename T>
  std::string toString(const T& value)
  {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  template <typename T, typename K>
  struct to_return_type {
    typedef T type;
  };

  template <typename T, typename K>
  T to(const K& value) {
    T result;
    std::stringstream ss;
    ss << value;
    ss >> result;
    return result;
  }

  std::string getJSONDate(time_t basetime = -1);
  void showStackTrace();

  bool isWhitespace(const std::string& str);
  bool isInteger(const std::string& str, bool unsignedOnly = false);

  // returns the current epoch time in miliseconds
  uint64_t getCurrentTime();

  std::string& toLowercase(std::string& str);
  std::string& toUppercase(std::string& str);
  std::string& trim(std::string& str);

  std::string dirname(const std::string& path);
  std::string basename(const std::string& path);
  std::string getExtension(const std::string& path);

  std::string resolvePath(size_t count, ...);

  template <typename T>
  size_t getOrderOfMagnitude(T nbr, size_t base = 10) {
    if (nbr < 0)
      nbr *= -1;
    size_t mag;
    for (mag = 0; nbr > 0; ++mag)
      nbr /= base;
    return std::pow(base, mag - 1);
  }

  std::string getCurrentWorkingDirectory();

  template <typename T>
  uint64_t hash(const T& value) {
    std::string str = toString(value);
    const char* ptr = str.c_str();
    // joaat hash
    uint64_t hash = 0;
    while (*ptr)
    {
      hash += *ptr;
      hash += (hash << 10);
      hash ^= (hash >> 6);
      ++ptr;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
  }

  std::string httpETag(const std::string& path, const size_t lastModified, const size_t size);

  std::string expandPath(const std::string& path);

  template <typename T>
  std::string strJoin(const std::vector<T>& vec, const std::string& delim = ", ") {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i)
    {
      ss << vec[i];
      if (i < vec.size() - 1)
        ss << delim;
    }
    return ss.str();
  }

  std::string encodeURIComponent(const std::string& str);
  std::string decodeURIComponent(const std::string& str);

  void showException(const std::string& msg, const std::exception& e);
}