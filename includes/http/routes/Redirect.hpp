#pragma once

#include "http/Route.hpp"

namespace HTTP {
  class ServerConfiguration;

  namespace Routes {

    class Redirect : public Route {
    public:
      Redirect(const YAML::Node& node, const ServerConfiguration* server);
      ~Redirect();
      Redirect(const Redirect& other);

      void handle(const Request& req, Response& res) const;

      virtual inline Redirect* clone() const {
        return new Redirect(this->node, this->server);
      }

      void init();

      inline bool supportsCascade() const {
        return true;
      }

      inline const std::string& getRedirectUri() const {
        return this->node["redirect"]["uri"].getValue();
      }
      bool isRedirectPartial() const;
      std::string buildRedirectPath(const Request& req) const;
      bool isRedirectPermanent() const;

    private:
      const YAML::Node& getNode() const;
    };
  }
}