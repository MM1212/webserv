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
  time_t t = time(0);
  tm* now = localtime(&t);
  std::ostringstream oss;
  // c++98
  oss << std::setfill('0') << std::setw(2) << now->tm_mday << " ";
  switch (now->tm_mon) {
    case 0:
      oss << "Jan";
      break;
    case 1:
      oss << "Feb";
      break;
    case 2:
      oss << "Mar";
      break;
    case 3:
      oss << "Apr";
      break;
    case 4:
      oss << "May";
      break;
    case 5:
      oss << "Jun";
      break;
    case 6:
      oss << "Jul";
      break;
    case 7:
      oss << "Aug";
      break;
    case 8:
      oss << "Sep";
      break;
    case 9:
      oss << "Oct";
      break;
    case 10:
      oss << "Nov";
      break;
    case 11:
      oss << "Dec";
      break;
  }
  oss << " " << (now->tm_year + 1900) << " ";
  oss << std::setfill('0') << std::setw(2) << now->tm_hour << ":";
  oss << std::setfill('0') << std::setw(2) << now->tm_min << ":";
  oss << std::setfill('0') << std::setw(2) << now->tm_sec << " GMT";
  return oss.str();
}