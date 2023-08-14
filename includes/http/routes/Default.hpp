#pragma once

#include <Yaml.hpp>
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/Route.hpp"
#include <Settings.hpp>

namespace HTTP {
  class ServerConfiguration;
  namespace Routes {
    class Default : public Route {
    public:
      Default(const YAML::Node& node, const ServerConfiguration* server);
      ~Default();
      Default(const Default& other);

      inline bool hasStatic() const {
        return this->getSettings().has("static");
      }
      inline bool hasCGI() const {
        return this->getSettings().has("cgi");
      }
      inline bool hasRedirect() const {
        return this->getSettings().has("redirect");
      }
      // static checks
      // will throw if not found
      inline const std::string& getRoot() const {
        return this->getSettings()["static"]["root"].getValue();
      }
      inline const std::string getIndex() const {
        try {
          return this->getSettings()["static"]["index"].getValue();
        }
        catch (const std::exception& e) {
          return Instance::Get<Settings>()->get<std::string>("http.static.default_index");
        }
      }
      inline bool hasDirectoryListing() const {
        try {
          return this->getSettings()["static"]["directory_listing"].as<bool>();
        }
        catch (const std::exception& e) {
          return false;
        }
      }
      inline bool ignoreHiddenFiles() const {
        try {
          return this->getSettings()["static"]["ignore_hidden"].as<bool>();
        }
        catch (const std::exception& e) {
          return true;
        }
      }
      // TODO: cgi checks
      // redirect checks
      inline const std::string& getRedirectUri() const {
        return this->getSettings()["redirect"]["uri"].getValue();
      }
      inline bool isRedirectPartial() const {
        if (!this->hasRedirect())
          return false;
        if (!this->getSettings()["redirect"].has("partial"))
          return false;
        return this->getSettings()["redirect"]["partial"].as<bool>();
      }
      inline std::string buildRedirectPath(const Request& req) const {
        if (!this->isRedirectPartial())
          return this->getRedirectUri();
        return this->getRedirectUri() + req.getPath().substr(0, this->getUri().size());
      }
      inline bool isRedirectPermanent() const {
        if (!this->hasRedirect())
          return false;
        if (!this->getSettings()["redirect"].has("type"))
          return true;
        return this->getSettings()["redirect"]["type"].getValue() == "permanent";
      }
      virtual void init(bool injectMethods = true);
    };
  }
}