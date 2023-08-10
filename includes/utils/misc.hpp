#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>
#include <cstdlib>
#include <stdarg.h>
#include <cmath>

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

  std::string getJSONDate(time_t basetime = -1);
  void showStackTrace();

  template <typename T>
  bool debugQuit(T* inst, void (T::* func)()) {
    static bool disabledSyncIO = false;
    if (!disabledSyncIO) {
      fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
      disabledSyncIO = true;
    }
    if (disabledSyncIO && std::cin.peek() == 'q')
    {
      (inst->*func)();
      return true;
    }
    return false;
  }

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
}