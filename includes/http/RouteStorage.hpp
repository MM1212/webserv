#pragma once

#include <utils/Instance.hpp>
#include <Yaml.hpp>

#include "http/Route.hpp"
#include "http/routes/all.hpp"

namespace HTTP {
  class ServerConfiguration;
  class RouteStorage {
  public:
    ~RouteStorage();

    template <typename T>
    T* buildRoute(const YAML::Node& node, const ServerConfiguration* server) const {
      return new T(node, server);
    }
  private:
    RouteStorage();

    friend class ::Instance;
  };
};