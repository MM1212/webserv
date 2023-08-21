/**
 * Redirect.hpp
 * Redirect Module for a Route.
 * It handles temporary & permanent redirections.
 * It also has support for partial redirections. (redirecting to a subpath of the requested path)
*/
#pragma once

#include "http/routing/Module.hpp"

namespace HTTP {
  namespace Routing {
    class Redirect : public Module {
    public:
      Redirect(const Route& route, const YAML::Node& node);
      ~Redirect();
      Redirect(const Redirect& other);

      inline Redirect* clone() const { return new Redirect(*this); }

      inline const std::string& getRedirectUri() const {
        return this->getSettings()["uri"].getValue();
      }
      bool isRedirectPartial() const;
      std::string buildRedirectPath(const Request& req) const;
      bool isRedirectPermanent() const;

      bool handle(const Request& req, Response& res) const;
    };
  }
}