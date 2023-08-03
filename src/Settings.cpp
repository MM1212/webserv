#include "Settings.hpp"
#include <utils/Logger.hpp>

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
    if (!this->config["max_connections"].is<int>())
      throw std::runtime_error("max_connections isn't an integer");
    if (!this->config["keep_alive_timeout"].is<int>())
      throw std::runtime_error("keep_alive_timeout isn't an integer");
    if (!this->config["max_body_size"].is<int>())
      throw std::runtime_error("max_body_size isn't an integer");
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

int Settings::getMaxConnections() const {
  static int cache = this->config["max_connections"].as<int>();
  return cache;
}

int Settings::getKeepAliveTimeout() const {
  static int cache = this->config["keep_alive_timeout"].as<int>();
  return cache;
}

int Settings::getMaxBodySize() const {
  static int cache = this->config["max_body_size"].as<int>();
  return cache;
}