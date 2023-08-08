#pragma once

#include "http/Route.hpp"

namespace HTTP {
  class ServerConfiguration;

  namespace Routes {

    class CGI : public Route {
    public:
      CGI(const YAML::Node& node, const ServerConfiguration* server)
        : Route(server, Types::CGI, node) {
        throw std::runtime_error("Not Implemented");
      }
      ~CGI() {}
      CGI(const CGI& other)
        : Route(other.server, other.type, other.node) {
        throw std::runtime_error("Not Implemented");
      }

      void handle(const Request& req, Response& res) const {
        (void)req;
        (void)res;
        throw std::runtime_error("Not Implemented");
      }
      CGI* clone() const {
        return new CGI(this->node, this->server);
      }
    };
  }
}