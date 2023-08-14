#include "http/routing/modules/Redirect.hpp"
#include "http/ServerConfiguration.hpp"

using namespace HTTP;
using namespace HTTP::Routing;

Redirect::Redirect(const Route& route, const YAML::Node& node)
  : Module(Types::Redirect, route, node) {
  this->init();
}

Redirect::~Redirect() {}

Redirect::Redirect(const Redirect& other) : Module(other) {
  this->init();
}


bool Redirect::isRedirectPartial() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("partial"))
    return this->getServer()->getDefaultRoute()->isRedirectPartial();
  return settings["partial"].as<bool>();
}

std::string Redirect::buildRedirectPath(const Request& req) const {
  if (!this->isRedirectPartial())
    return this->getRedirectUri();
  size_t pos = req.getPath().find(this->route.getUri());
  if (pos == std::string::npos)
    return this->getRedirectUri();
  std::string uriHost = req.getPath();
  uriHost.erase(pos, this->route.getUri().size());
  if (uriHost.empty())
    uriHost = "/";
  uriHost = Utils::resolvePath(2, this->getRedirectUri().c_str(), uriHost.c_str());
  return uriHost;
}

bool Redirect::isRedirectPermanent() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("type"))
    return this->getServer()->getDefaultRoute()->isRedirectPermanent();
  return settings["type"].getValue() == "permanent";
}


bool Redirect::handle(const Request& req, Response& res) const {
  res.redirect(this->buildRedirectPath(req), this->isRedirectPermanent());
  return true;
}