#pragma once

#include <Yaml.hpp>
#include <utils/Instance.hpp>

class Settings {
  static const std::string path;
public:
  int getMaxConnections() const;
  int getKeepAliveTimeout() const;
  int getMaxBodySize() const;
  bool isValid() const;
private:
  Settings();
  const YAML::Node config;

  friend class Instance;
};