#include <Yaml.hpp>
#include <shared.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>
#include <Settings.hpp>
#include <http/ServerManager.hpp>
#include <lua.hpp>

static Settings* settings = Instance::Get<Settings>();

int main(int ac, char** av, char** env) {
  ac--;
  av++;
  if (!settings->isValid())
    return 1;
  YAML::RunTests();
  HTTP::ServerManager* serverManager = Instance::Get<HTTP::ServerManager>();
  try {
    if (!serverManager->loadConfig(ac > 0 ? av[0] : settings->get<std::string>("misc.default_config_file")))
      return 1;
  }
  catch (const std::exception& e) {
    Utils::showException("Failed to load config file", e);
    return 1;
  }
  serverManager->setEnv(env);
  try {
    serverManager->bindServers();
  }
  catch (const std::exception& e) {
    Utils::showException("Failed to bind servers blocks", e);
    return 1;
  }
  try {
    serverManager->run();
  }
  catch (const std::exception& e) {
    Utils::showException("Failed to run servers or an exception ocurred while executing", e);
    return 1;
  }
  return 0;
}