/**
 * Settings.hpp
 * Settings class using Instance that loads & interprets a YAML settings file.
 * This is useful for storing settings that are used everywhere, like HTTP status codes.
 * Has some helper methods for mime types & status codes, everything else goes through get<T>.
*/
#pragma once

#include <Yaml.hpp>
#include <utils/Instance.hpp>
#include <utils/misc.hpp>
#include <shared.hpp>

class Settings {
  static const char* path;
public:
  bool isValid() const;

  template <typename T>
  T get(const std::string& path) const {
    std::vector<std::string> parts = Utils::split(path, ".");
    const YAML::Node* node = &this->config;
    for (
      size_t i = 0;
      i < parts.size() && node->isValid();
      ++i
      ) {
      const std::string& key = parts[i];
      if (
        Utils::isInteger(key, true) &&
        node->is<YAML::Types::Sequence>() &&
        node->has(std::atoi(key.c_str())))
        node = &node->get(std::atoi(key.c_str()));
      else
        node = &node->get(key);
    }
    return node->as<T>();
  }
  const std::string httpStatusCode(int code) const;
  const std::string& httpMimeType(const std::string& ext) const;
private:
  Settings();
  const YAML::Node config;

  friend class Instance;
};