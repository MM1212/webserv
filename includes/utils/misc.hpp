#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

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
}