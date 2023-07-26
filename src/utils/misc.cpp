#include "utils/misc.hpp"
#include <execinfo.h>

std::vector<std::string> Utils::split(const std::string& str, std::string delim)
{
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  std::string copy(str);
  while ((pos = copy.find(delim)) != std::string::npos) {
    token = copy.substr(0, pos);
    tokens.push_back(token);
    copy.erase(0, pos + delim.length());
  }
  tokens.push_back(copy);
  return tokens;
}
// formats time in this format Fri, 21 Jul 2023 19:27:34 GMT
std::string Utils::getJSONDate()
{
  std::string result;
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = gmtime(&rawtime);

  strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
  result = buffer;
  return result;
}

void Utils::showStackTrace() {
  void* buff[10];
  int size = backtrace(buff, 10);
  char** symbols = backtrace_symbols(buff, size);
  std::cerr << "Stack trace:" << std::endl;
  for (int i = 0; i < size; i++)
  {
    std::cerr << symbols[i] << std::endl;
  }
}