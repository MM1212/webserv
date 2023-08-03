#include "utils/Logger.hpp"

using namespace Logger;

const Stream Logger::debug("DEBUG", CYAN);
const Stream Logger::info("INFO", BLUE);
const Stream Logger::success("SUCCESS", GREEN);
const Stream Logger::warning("WARNING", ORANGE);
const Stream Logger::error(std::cerr, "ERROR", RED);

Stream::Stream(
  std::ostream& target,
  const std::string& header,
  const std::string& color)
  : header(header), target(target), color(color) {}
Stream::Stream(
  const std::string& header,
  const std::string& color)
  : header(header), target(std::cout), color(color) {}

Stream::~Stream() {}

Stream::Stream(const Stream& other)
  : header(other.header), target(other.target),
  color(other.color) {}

Stream& Stream::operator=(const Stream& other) {
  if (this == &other) return *this;
  this->header = other.header;
  this->color = other.color;
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