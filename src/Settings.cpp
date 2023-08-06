#include "Settings.hpp"
#include <utils/Logger.hpp>
#include <utils/misc.hpp>

const std::string Settings::path = "config/bin/settings.yaml";

Settings::Settings() : config() {
  try {
    const_cast<YAML::Node&>(this->config) = YAML::LoadFile(Settings::path);
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load settings: "
      << Logger::param(e.what())
      << std::endl;
  }
}

bool Settings::isValid() const {
  try {
    if (!this->config.isValid())
      throw std::runtime_error("config file isn't valid");
    if (!this->config.is<YAML::Types::Map>())
      throw std::runtime_error("config file isn't a map");
    if (!this->config["socket"].is<YAML::Types::Map>())
      throw std::runtime_error("socket isn't a map");
    if (!this->config["socket"]["max_connections"].is<int>())
      throw std::runtime_error("max_connections isn't an integer");
    if (!this->config["socket"]["keep_alive_timeout"].is<int>())
      throw std::runtime_error("keep_alive_timeout isn't an integer");
    if (!this->config["socket"]["read_buffer_size"].is<int>())
      throw std::runtime_error("read_buffer_size isn't an integer");
    if (!this->config["http"].is<YAML::Types::Map>())
      throw std::runtime_error("http isn't a map");
    if (!this->config["http"]["max_body_size"].is<int>())
      throw std::runtime_error("max_body_size isn't an integer");
    if (!this->config["http"]["max_uri_size"].is<int>())
      throw std::runtime_error("max_uri_size isn't an integer");
    if (!this->config["http"]["status_codes"].is<YAML::Types::Map>())
      throw std::runtime_error("status_codes isn't a map");
    if (!this->config["misc"].is<YAML::Types::Map>())
      throw std::runtime_error("misc isn't a map");
    if (!this->config["misc"]["log_level"].is<int>())
      throw std::runtime_error("log_level isn't an integer");
    return true;
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load settings: "
      << Logger::param(e.what())
      << std::endl;
    return false;
  }
}

const std::string Settings::httpStatusCode(int code) const {
  static const YAML::Node& codes = this->config["http"]["status_codes"];
  const std::string codeStr = Utils::toString(code);
  if (!codes.has(codeStr))
    return "";
  return codes[codeStr].as<std::string>();
}