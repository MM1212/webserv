/**
 * Logger.hpp
 * Logging utility using streams.
 * If you want a parameter highlighted, use the Logger::param() method.
 * Settings stores a log level which might disable some logs.
*/
#pragma once

#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

// COLORS
#define RESET	"\033[39m"
#define BLACK	"\033[30m"
#define DARK_RED	"\033[31m"
#define DARK_GREEN	"\033[32m"
#define DARK_YELLOW	"\033[33m"
#define DARK_BLUE	"\033[34m"
#define DARK_MAGENTA	"\033[35m"
#define DARK_CYAN	"\033[36m"
#define LIGHT_GRAY	"\033[37m"
#define DARK_GRAY	"\033[90m"
#define RED	"\033[91m"
#define GREEN	"\033[92m"
#define ORANGE	"\033[93m"
#define BLUE	"\033[94m"
#define MAGENTA	"\033[95m"
#define CYAN	"\033[96m"
#define WHITE	"\033[97m"

#include <cstring>
#include <cerrno>


namespace Logger {

  template <typename T>
  std::string param(const T& value) {
    std::stringstream ss;
    ss << ORANGE << value << RESET;
    return ss.str();
  }

  inline std::string errstr() {
    return param(std::strerror(errno));
  }

  class Stream {
  private:
    std::string header;
    std::ostream& target;
    std::string color;
    bool enabled;
    std::stringstream vstream;
  public:
    Stream(
      std::ostream& target,
      const std::string& header,
      const std::string& color,
      bool enabled = true
    );
    Stream(
      const std::string& header,
      const std::string& color,
      bool enabled = true
    );
    ~Stream();
    Stream(const Stream& other);
    Stream& operator=(const Stream& other);

    template <typename T>
    std::ostream& operator<<(const T& value) {
      this->vstream.str("");
      if (!this->enabled) return this->vstream;
      return this->target << this->buildHeader() << " " << value;
    }

    template <typename T>
    std::string operator>>(const T& value) const {
      return Logger::param(value);
    }

  private:
    std::string getTime() const;
    std::string buildHeader() const;
  };



  extern Stream debug;
  extern Stream info;
  extern Stream success;
  extern Stream warning;
  extern Stream error;

  extern Stream child;
};
