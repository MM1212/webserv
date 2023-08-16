#pragma once

#include <string>
#include <Yaml.hpp>
#include <iostream>

#include "types.hpp"
#include "http/Methods.hpp"


namespace HTTP {
  class Route;
  class ServerConfiguration;
  class Request;
  class Response;
  namespace Routing {
    class Module {
    public:
      static Module* Create(const std::string& type, const Route& route, const YAML::Node& node);
      static Module* Create(const Types::Type type, const Route& route, const YAML::Node& node);
    public:
      Module(const Types::Type type, const Route& route, const YAML::Node& node);
      virtual ~Module() = 0;
      Module(const Module& other);

      inline Types::Type getType() const { return this->type; }
      inline const Route& getRoute() const { return this->route; }
      inline const YAML::Node& getSettings() const { return this->node["settings"]; }
      inline YAML::Node& getSettings() { return const_cast<YAML::Node&>(this->node["settings"]); }
      const ServerConfiguration* getServer() const;
      virtual inline bool supportsExpect() const { return false; }
      // not related to 405.
      // if returns false, it will skip to the next module
      bool isMethodAllowed(Methods::Method method) const;

      virtual Module* clone() const = 0;

      virtual bool handle(const Request& req, Response& res) const = 0;
      virtual bool next(Response& res, int statusCode = -1) const;
    protected:
      const Types::Type type;
      const Route& route;
      const YAML::Node& node;

      virtual void init();

      Middleware getNoMatch() const;

    public:
      friend std::ostream& operator<<(std::ostream& os, const Module& module);
    };

  }
}