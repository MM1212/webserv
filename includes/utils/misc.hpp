#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <fcntl.h>

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

  std::string getJSONDate();
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
}