#pragma once

#include <string>
#include <Yaml.hpp>
#include "Methods.hpp"
#include <utils/misc.hpp>

#include <iostream>

namespace HTTP {
  class ServerConfiguration;
  class Request;
  class Response;
  class Route {
  public:
    struct Types {
      enum Type {
        None = -1,
        Default,
        Static,
        Redirect,
        CGI
      };
      static std::string ToString(Type type);
    };
  public:
    Route(const ServerConfiguration* server, Types::Type type, const YAML::Node& node);
    virtual ~Route() = 0;
    Route(const Route& other);
    Types::Type getType() const;
    const std::string& getPath() const;
    bool hasErrorPage(int code) const;
    const std::string& getErrorPage(int code) const;
    int getMaxBodySize() const;

    bool isMethodAllowed(Methods::Method method) const;
    virtual inline bool supportsCascade() const {
      return false;
    }

    virtual inline bool supportsExpect() const {
      return false;
    }

    virtual void handle(const Request& req, Response& res) const = 0;
    virtual Route* clone() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Route& route);
    friend std::ostream& operator<<(std::ostream& os, const Route* route);
  protected:
    const ServerConfiguration* server;
    const YAML::Node& node;
    Types::Type type;

    virtual void init();
  };
}