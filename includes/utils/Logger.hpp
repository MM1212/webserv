#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <stdint.h>

# define COLORS_RESET "\033[0m"
# define COLORS_RED "\033[0;31m"
# define COLORS_GREEN "\033[0;32m"
# define COLORS_YELLOW "\033[0;33m"
# define COLORS_BLUE "\033[0;34m"
# define COLORS_MAGENTA "\033[0;35m"
# define COLORS_CYAN "\033[0;36m"
# define COLORS_WHITE "\033[0;37m"
# define COLORS_LIGHT_RED "\033[1;31m"
# define COLORS_LIGHT_GREEN "\033[1;32m"
# define COLORS_LIGHT_YELLOW "\033[1;33m"
# define COLORS_LIGHT_BLUE "\033[1;34m"
# define COLORS_LIGHT_MAGENTA "\033[1;35m"
# define COLORS_LIGHT_CYAN "\033[1;36m"
# define COLORS_LIGHT_WHITE "\033[1;37m"

namespace Utils {

class Logger {
public:
  struct Levels {
    enum Level {
      Debug,
      Info,
      Warn,
      Error
    };
  };
  struct Config {
    uint32_t level;
    std::string file;
  };
  struct Color {
    std::string code;
    std::string label;
    Levels::Level level;
  };
public:
  Logger(const std::string& env, const Config& config);
  Logger(const std::string& env);
  ~Logger();
  Logger(const Logger& other);
  Logger& operator=(const Logger& other);
  void log(const Levels::Level& level, const std::string fmt, ...);
  void debug(const std::string fmt, ...);
  void info(const std::string fmt, ...);
  void warn(const std::string fmt, ...);
  void error(const std::string fmt, ...);

  Logger extend(const std::string& env) const;
private:
  static const Color colors[4];

  Logger();
  std::ofstream file;
  bool fileReady;
  Config config;
  std::string env;
  std::string getTimestamp() const;
  const std::string& getEnv() const;
  std::string getHeader(const Color& color);
  const Color& getColor(const Levels::Level level) const;
  void init();
  void doLog(const Levels::Level& level, const std::string& fmt, va_list args);
  void logToFile(const std::string& str);
};
}