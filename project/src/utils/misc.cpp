#include "utils/misc.hpp"
#include <utils/Logger.hpp>
#include <execinfo.h>
#include <unistd.h>

std::vector<std::string> Utils::split(const std::string& str, std::string delim)
{
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  std::string copy(str);
  while ((pos = copy.find(delim)) != std::string::npos) {
    token = copy.substr(0, pos);
    if (token.size() > 0)
      tokens.push_back(token);
    copy.erase(0, pos + delim.length());
  }
  if (copy.size() > 0)
    tokens.push_back(copy);
  return tokens;
}

std::string& Utils::capitalize(std::string& str)
{
  if (str.length() > 0)
    str[0] = std::toupper(str[0]);
  return str;
}

// formats time in this format Fri, 21 Jul 2023 19:27:34 GMT
std::string Utils::getJSONDate(time_t basetime /* = -1 */)
{
  std::string result;
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[80];

  if (basetime != -1)
    rawtime = basetime;
  else
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
  std::cerr << "Stack trace:" << std::newl;
  for (int i = 0; i < size; i++)
  {
    std::cerr << symbols[i] << std::newl;
  }
}

bool Utils::isWhitespace(const std::string& str) {
  for (size_t i = 0; i < str.size(); i++)
    if (!std::isspace(str[i]))
      return false;
  return true;
}

bool Utils::isInteger(const std::string& str, bool unsignedOnly) {
  if (str.empty())
    return false;
  size_t i = 0;
  if (str[0] == '-' || str[0] == '+') {
    if (unsignedOnly)
      return false;
    i++;
  }
  if (i == str.size())
    return false;
  for (; i < str.size(); i++)
    if (!std::isdigit(str[i]))
      return false;
  return true;
}

uint64_t Utils::getCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}

std::string& Utils::trim(std::string& str) {
  static const std::string whitespace = " \t\n\r\f\v";
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos) {
    str.clear();
    return str;
  }
  str.erase(0, start);
  size_t end = str.find_last_not_of(whitespace);
  str.erase(end + 1);
  return str;
}

std::string& Utils::toLowercase(std::string& str) {
  for (size_t i = 0; i < str.size(); i++)
    str[i] = std::tolower(str[i]);
  return str;
}

std::string& Utils::toUppercase(std::string& str) {
  for (size_t i = 0; i < str.size(); i++)
    str[i] = std::toupper(str[i]);
  return str;
}

std::string Utils::dirname(const std::string& path) {
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos)
    return path;
  std::string dir = path.substr(0, pos);
  if (dir.empty())
    return "/";
  return dir;
}

std::string Utils::basename(const std::string& path) {
  size_t pos = path.find_last_of('/');
  if (pos == std::string::npos)
    return path;
  return path.substr(pos + 1);
}

std::string Utils::getExtension(const std::string& path) {
  size_t pos = path.find_last_of('.');
  if (pos == std::string::npos)
    return "";
  return path.substr(pos + 1);
}

// joins all the paths correctly missing/more than /
std::string Utils::resolvePath(size_t count, ...) {
  va_list args;
  va_start(args, count);
  std::string result;
  for (size_t i = 0; i < count; i++) {
    std::string part(va_arg(args, char*));
    if (result.empty())
      result = part;
    else {
      uint64_t pos;
      while ((pos = result.find("/./")) != std::string::npos)
        result.erase(pos, 2);
      while ((pos = result.find("/../")) != std::string::npos) {
        if (pos == 0)
          result.erase(0, 3);
        else {
          uint64_t prev = result.find_last_of('/', pos - 1);
          result.erase(prev, pos - prev + 3);
        }
      }
      if (result[result.size() - 1] != '/' && part[0] != '/')
        result += '/';
      else if (result[result.size() - 1] == '/' && part[0] == '/')
        part.erase(0, 1);
      result += part;
    }
  }
  return result;
}

std::string Utils::getCurrentWorkingDirectory() {
  char* cwd = getcwd(NULL, 0);
  std::string path(cwd);
  free(cwd);
  return path;
}
std::string Utils::httpETag(const std::string& path, const size_t lastModified, const size_t size)
{
  std::stringstream ss;
  ss << path << "-" << lastModified << "-" << size;
  uint64_t hash = Utils::hash(ss.str());
  std::stringstream ss2;
  ss2 << std::hex << hash;
  return ss2.str();
}

std::string Utils::expandPath(const std::string& path) {
  uint64_t pos;
  std::string result = path;
  std::string home;
  const char* homePtr = getenv("HOME");
  if (!homePtr)
    home = "/root";
  else
    home = homePtr;
  while ((pos = result.find_first_of('~')) != std::string::npos) {
    result = Utils::resolvePath(3,
      result.substr(0, pos).c_str(),
      home.c_str(),
      result.substr(pos + 1).c_str()
    );
  }
  return result;
}

std::string Utils::encodeURIComponent(const std::string& str) {
  std::string result;
  for (size_t i = 0; i < str.size(); i++) {
    if (std::isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~')
      result += str[i];
    else {
      std::stringstream buf;
      buf << std::hex << (int)str[i];
      result += '%' + buf.str();
    }
  }
  return result;
}

std::string Utils::decodeURIComponent(const std::string& str) {
  std::string result;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%') {
      if (i + 2 >= str.size())
        throw std::runtime_error("Invalid URI");
      std::stringstream buf;
      buf << std::hex << str[i + 1] << str[i + 2];
      int c;
      buf >> c;
      result += (char)c;
      i += 2;
    }
    else
      result += str[i];
  }
  return result;
}


void Utils::showException(const std::string& msg, const std::exception& e) {
  std::stringstream ss;
  ss << msg << ": " << Logger::param(e.what());
  if (errno != 0)
    ss << " (" << Logger::param(strerror(errno)) << ")";
  ss << std::newl;
  Logger::error << ss.str();
}