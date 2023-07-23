#include "utils/misc.hpp"

std::vector<std::string> Utils::split(const std::string& str, std::string delim)
{
  std::vector<std::string> result;
  size_t pos = 0;
  std::string token;
  std::string copy(str);
  while ((pos = copy.find(delim)) != std::string::npos) {
    token = str.substr(0, pos);
    result.push_back(token);
    copy.erase(0, pos + delim.length());
  }
  result.push_back(str);
  return result;
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