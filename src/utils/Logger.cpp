#include "utils/Logger.hpp"
#include <Settings.hpp>

using namespace Logger;

static const Settings* settings = Instance::Get<Settings>();
static const int logLevel = settings->get<int>("misc.log_level");

Stream Logger::debug("DEBUG", CYAN, logLevel > -1 && logLevel <= 0);
Stream Logger::info("INFO", BLUE, logLevel > -1 && logLevel <= 1);
Stream Logger::success("SUCCESS", GREEN, logLevel > -1 && logLevel <= 2);
Stream Logger::warning("WARNING", ORANGE, logLevel > -1 && logLevel <= 3);
Stream Logger::error(std::cerr, "ERROR", RED, logLevel > -1 && logLevel <= 4);
Stream Logger::child(std::cerr, "CHILD", LIGHT_GRAY, true);

Stream::Stream(
  std::ostream& target,
  const std::string& header,
  const std::string& color,
  bool enabled
) : header(header), target(target), color(color), enabled(enabled) {}
Stream::Stream(
  const std::string& header,
  const std::string& color,
  bool enabled
) : header(header), target(std::cout), color(color), enabled(enabled) {}

Stream::~Stream() {}

Stream::Stream(const Stream& other)
  : header(other.header), target(other.target),
  color(other.color), enabled(other.enabled) {}

Stream& Stream::operator=(const Stream& other) {
  if (this == &other) return *this;
  this->header = other.header;
  this->color = other.color;
  this->enabled = other.enabled;
  return *this;
}

std::string Stream::getTime() const {
  std::stringstream stream;
  time_t now = time(0);
  tm* ltm = localtime(&now);

  stream << std::setw(2) << std::setfill('0') << ltm->tm_mday << "/";
  stream << std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << "/";
  stream << std::setw(4) << std::setfill('0') << 1900 + ltm->tm_year << "|";
  stream << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":";
  stream << std::setw(2) << std::setfill('0') << ltm->tm_min << ":";
  stream << std::setw(2) << std::setfill('0') << ltm->tm_sec;
  return stream.str();
}

std::string Stream::buildHeader() const {
  std::stringstream ss;
  ss << "[" RED
    << this->getTime()
    << RESET << "] "
    << "[" << this->color
    << this->header
    << RESET << "]";
  return ss.str();
}