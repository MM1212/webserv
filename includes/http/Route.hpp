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
    int getMaxBodySize() const;
    bool isMethodAllowed(Methods::Method method) const;

    void handle(const Request& req, Response& res) const;

    friend std::ostream& operator<<(std::ostream& os, const Route& route);
    virtual void init(bool injectMethods = true);
  protected:
    const ServerConfiguration* server;
    const YAML::Node& node;
    std::vector<Routing::Module*> modules;

    void addModule(Routing::Module* module);
    void initModule(const YAML::Node& node);

    friend class Routing::Module;
  };
}