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

      void handle(const Request& req, Response& res) const;
      inline Default* clone() const {
        return new Default(*this);
      }

      inline bool hasStatic() const {
        return this->node.has("static");
      }
      inline bool hasCGI() const {
        return this->node.has("cgi");
      }
      inline bool hasRedirect() const {
        return this->node.has("redirect");
      }
      // static checks
      // will throw if not found
      inline const std::string& getRoot() const {
        return this->node["static"]["root"].getValue();
      }
      inline const std::string getIndex() const {
        try {
          return this->node["static"]["index"].getValue();
        }
        catch (const std::exception& e) {
          return Instance::Get<Settings>()->get<std::string>("http.static.default_index");
        }
      }
      inline bool hasDirectoryListing() const {
        try {
          return this->node["static"]["directory_listing"].as<bool>();
        }
        catch (const std::exception& e) {
          return false;
        }
      }
      // TODO: cgi checks
      // redirect checks
      inline const std::string& getRedirectUri() const {
        return this->node["redirect"]["uri"].getValue();
      }
      inline bool isRedirectPartial() const {
        if (!this->node.has("redirect"))
          return false;
        if (!this->node["redirect"].has("partial"))
          return false;
        return this->node["redirect"]["partial"].as<bool>();
      }
      inline std::string buildRedirectPath(const Request& req) const {
        if (!this->isRedirectPartial())
          return this->getRedirectUri();
        return this->getRedirectUri() + req.getPath().substr(0, this->getPath().size());
      }
      inline bool isRedirectPermanent() const {
        if (!this->node.has("redirect"))
          return false;
        if (!this->node["redirect"].has("type"))
          return true;
        return this->node["redirect"]["type"].getValue() == "permanent";
      }
    private:
      virtual void init();
    };
  }
}