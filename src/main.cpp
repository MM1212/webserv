#include <Yaml.hpp>
#include <webserv.hpp>
#include <shared.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>
#include <cstdlib>
#include <string.h>
#include <Settings.hpp>
#include <http/ServerManager.hpp>

static Settings* settings = Instance::Get<Settings>();

int main(int ac, char** av) {
  ac--;
  av++;
  if (!settings->isValid())
    return 1;
  YAML::RunTests();
  try {
    Instance::Get<HTTP::ServerManager>()->run();
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load config file: "
      << Logger::param(e.what())
      << " | "
      << Logger::param(strerror(errno))
      << std::endl;
    return 1;
  }

  return 0;
}