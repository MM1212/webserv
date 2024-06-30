/**
 * Route.hpp
 * Stores a route configuration for a server.
 * A route is a path that can be accessed by a client.
 * It stores multiple modules that will handle the request as they fit.
 * If n module doesn't handle the request (and the configuration allows), it will bounce the request to n + 1 module.
 * If no module handles the request, it will send a 404 Not Found.
 * If a module handles the request but the default protections are not met, it will send the appropriate error code (or bounce to the next one).
*/

#pragma once

#include <string>
#include <Yaml.hpp>
#include "Methods.hpp"
#include "routing/routing.hpp"
#include <utils/misc.hpp>

#include <iostream>

namespace HTTP {
  class ServerConfiguration;
  class Request;
  class Response;
  namespace Routes {
    class Default;
  }
  class Route {
  public:
    typedef Routing::Types Types;
  public:
    Route(const ServerConfiguration* server, const YAML::Node& node);
    virtual ~Route();
    Route(const Route& other);
    inline const std::string& getUri() const { return this->node["uri"].getValue(); }

    inline const YAML::Node& getSettings() const { return this->node["settings"]; }
    // settings quick getters
    bool hasErrorPage(int code) const;
    const std::string& getErrorPage(int code) const;
    uint32_t getMaxBodySize() const;
    bool isMethodAllowed(Methods::Method method) const;

    void handle(const Request& req, Response& res) const;
    Routing::Module* getModule(const Routing::Types::Type type) const;

    friend std::ostream& operator<<(std::ostream& os, const Route& route);
    virtual void init(bool injectMethods = true);
  protected:
    const ServerConfiguration* server;
    const YAML::Node& node;
    std::vector<Routing::Module*> modules;

    void addModule(Routing::Module* module);
    void initModule(const YAML::Node& node);

    friend class Routing::Module;
    friend class Routes::Default;
  };
}