#pragma once

#include <Yaml.hpp>
#include <utils/Instance.hpp>
#include <utils/misc.hpp>

class Settings {
  static const std::string path;
public:
  int getMaxConnections() const;
  int getKeepAliveTimeout() const;
  int getMaxBodySize() const;
  bool isValid() const;

  template <typename T>
  T get(const std::string& path) const {
    std::vector<std::string> parts = Utils::split(path, ".");
    YAML::Node node = this->config;
    for (
      size_t i = 0;
      i < parts.size() && node.isValid();
      ++i
      ) {
      YAML::Node part = node[parts[i]];
      node = part;
    }
    return node.as<T>();
  }
  const std::string statusCode(int code) const;
private:
  Settings();
  const YAML::Node config;

  friend class Instance;
};