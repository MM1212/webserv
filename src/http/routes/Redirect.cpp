#include "http/routes/Redirect.hpp"
#include "http/ServerConfiguration.hpp"

using namespace HTTP;
using namespace HTTP::Routes;

Redirect::Redirect(const YAML::Node& node, const ServerConfiguration* server)
  : Route(server, Types::Redirect, node) {
  this->init();
}

Redirect::~Redirect() {}

Redirect::Redirect(const Redirect& other)
  : Route(other) {
  this->init();
}

const YAML::Node& Redirect::getNode() const {
  return this->node["redirect"];
}

void Redirect::init() {
  this->Route::init();
}

bool Redirect::isRedirectPartial() const {
  const YAML::Node& redir = this->getNode();
  if (!redir.has("partial"))
    return this->server->getDefaultRoute()->isRedirectPartial();
  return redir["partial"].as<bool>();
}

std::string Redirect::buildRedirectPath(const Request& req) const {
  (void)req;
  if (!this->isRedirectPartial())
    return this->getRedirectUri();
  size_t pos = req.getPath().find(this->getPath());
  if (pos == std::string::npos)
    return this->getRedirectUri();
  std::string uriHost = req.getPath();
  uriHost.erase(pos, this->getPath().size());
  if (uriHost.empty())
    uriHost = "/";
  uriHost = Utils::resolvePath(2, this->getRedirectUri().c_str(), uriHost.c_str());
  return uriHost;
}

bool Redirect::isRedirectPermanent() const {
  const YAML::Node& redir = this->getNode();
  if (!redir.has("type"))
    return this->server->getDefaultRoute()->isRedirectPermanent();
  return redir["type"].getValue() == "permanent";
}


void Redirect::handle(const Request& req, Response& res) const {
  res.redirect(this->buildRedirectPath(req), this->isRedirectPermanent());
}