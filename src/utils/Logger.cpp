#include "utils/Logger.hpp"

using Utils::Logger;

const Logger::Color Logger::colors[4] = {
  { COLORS_CYAN, "DEBUG", Logger::Levels::Debug },
  { COLORS_LIGHT_BLUE, "INFO", Logger::Levels::Info },
  { COLORS_LIGHT_YELLOW, "WARN", Logger::Levels::Warn },
  { COLORS_RED, "ERROR", Logger::Levels::Error }
};

Logger::Logger() {}

Logger::Logger(const std::string& _env, const Config& _config)
  : file(), fileReady(false), config(_config), env(_env) {
  this->init();
}

Logger::Logger(const std::string& _env)
  : file(), fileReady(false), config((Config) { .level = 0, .file = "" }), env(_env) {
  this->init();
}

Logger::Logger(const Logger& other)
  : file(), fileReady(false), config(other.config), env(other.env) {
  this->init();
}

Logger& Logger::operator=(const Logger& other) {
  if (this == &other) return *this;
  if (this->file.is_open())
    this->file.close();
  this->fileReady = false;
  this->config = other.config;
  this->env = other.env;
  this->init();
  return *this;
}

Logger::~Logger() {
  if (this->file.is_open()) {
    this->file.close();
  }
}

const std::string& Logger::getEnv() const {
  return this->env;
}



std::string Logger::getTimestamp() const {
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

const Logger::Color& Logger::getColor(const Levels::Level level) const {
  for (
    uint32_t i = 0;
    i < 4;
    i++
    ) {
    if (this->colors[i].level == level)
      return this->colors[i];
  }
  throw std::runtime_error("Whoops!");
}

static void replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, start_pos + to.length(), to);
    start_pos += to.length();
  }
}

std::string Logger::getHeader(const Color& color) {
  std::stringstream stream;

  std::stringstream envStream;
  std::string envFormatted(this->env);
  envStream
    << COLORS_LIGHT_YELLOW
    << "/"
    << COLORS_RESET;
  replace(envFormatted, "/", envStream.str());
  stream
    << COLORS_RESET
    << "[" << COLORS_RED
    << this->getTimestamp()
    << COLORS_RESET << "] "
    << "[" << COLORS_WHITE
    << envFormatted
    << COLORS_RESET << "] "
    << "[" << color.code
    << color.label
    << COLORS_RESET << "] ";

  return stream.str();
}

void Logger::init() {
  if (this->config.file != "")
    this->file.open(this->config.file.c_str());
  this->fileReady = this->file.is_open();
}

void Logger::logToFile(const std::string& str) {
  if (!this->fileReady) return;
  this->file << str << std::endl;
}

// manually parse fmt
void Logger::doLog(const Levels::Level& level, const std::string& fmt, va_list args) {
  if (level > this->config.level) return;
  const Color& color = this->getColor(level);
  std::stringstream stream;
  stream << this->getHeader(color);

  std::string fmtCopy(fmt);
  size_t pos = 0;
  std::string token;
  while ((pos = fmtCopy.find("%")) != std::string::npos) {
    token = fmtCopy.substr(0, pos);
    stream << token;
    fmtCopy.erase(0, pos + 1);
    switch (fmtCopy[0]) {
    case 'd':
      stream << va_arg(args, int);
      break;
    case 'f':
      stream << va_arg(args, double);
      break;
    case 's':
      stream << va_arg(args, char*);
      break;
    case 'c':
      stream << va_arg(args, int);
      break;
    default:
      stream << "%" << fmtCopy[0];
      break;
    }
    fmtCopy.erase(0, 1);
  }
  stream << fmtCopy;
  std::string str(stream.str());
  std::cout << str << std::endl;
  this->logToFile(str);
}

void Logger::log(const Levels::Level& level, const std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->doLog(level, fmt, args);
  va_end(args);
}

void Logger::debug(const std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->doLog(Levels::Debug, fmt, args);
  va_end(args);
}

void Logger::info(const std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->doLog(Levels::Info, fmt, args);
  va_end(args);
}

void Logger::warn(const std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->doLog(Levels::Warn, fmt, args);
  va_end(args);
}

void Logger::error(const std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->doLog(Levels::Error, fmt, args);
  va_end(args);
}

Logger Logger::extend(const std::string& env) const {
  return Logger(this->env + "/" + env, this->config);
}