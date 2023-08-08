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

void Redirect::init() {}

bool Redirect::isRedirectPartial() const {
  const YAML::Node& redir = this->getNode();
  if (!redir.has("partial"))
    return this->server->getDefaultRoute()->isRedirectPartial();
  return redir["partial"].as<bool>();
}

std::string Redirect::buildRedirectPath(const Request& req) const {
  if (!this->isRedirectPartial())
    return this->getRedirectUri();
  return this->getRedirectUri() + req.getPath().substr(0, this->getPath().size());
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