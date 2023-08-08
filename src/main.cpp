#include <Yaml.hpp>
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
    HTTP::ServerManager* serverManager = Instance::Get<HTTP::ServerManager>();
    if (serverManager->loadConfig(ac > 0 ? av[0] : settings->get<std::string>("misc.default_config_file"))) {
      serverManager->bindServers();
      serverManager->run();
    }
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