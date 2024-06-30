#include "http/routing/Module.hpp"
#include "http/Route.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

using namespace HTTP::Routing;

Module::Module(const Types::Type type, const Route& route, const YAML::Node& node)
  : type(type), route(route), node(node) {}

Module::Module(const Module& other)
  : type(other.type), route(other.route), node(other.node) {}

Module::~Module() {}

void Module::init() {
  YAML::Node& node = const_cast<YAML::Node&>(this->node);
  if (!node.has("settings"))
    node.insert(YAML::Node::NewMap("settings"));
  const YAML::Node& settings = this->getSettings();
  if (!settings.is<YAML::Types::Map>())
    throw std::runtime_error("Settings must be a map");
  if (settings.has("methods") && !settings["methods"].is<YAML::Types::Sequence>())
    throw std::runtime_error("Methods must be a sequence");
}

Middleware Module::getNoMatch() const {
  if (!this->node.has("no_match")) return Middleware(Middleware::Next);
  if (this->node["no_match"].is<int>())
    return Middleware(this->node["no_match"].as<int>());
  Middleware::Type type = Middleware::FromString(this->node["no_match"].as<std::string>());
  return Middleware(type);
}

const HTTP::ServerConfiguration* Module::getServer() const {
  return this->route.server;
}

bool Module::isMethodAllowed(Methods::Method method) const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("methods") || !settings["methods"].is<YAML::Types::Sequence>())
    return true;
  const YAML::Node& methods = settings["methods"];
  for (uint64_t i = 0; i < methods.size(); ++i) {
    if (Methods::FromString(methods[i].getValue()) == method)
      return true;
  }
  return false;
}

bool Module::next(Response& res, int statusCode /* = -1 */) const {
  const Middleware noMatch = this->getNoMatch();
  if (statusCode == -1) {
    switch (static_cast<int>(noMatch)) {
    case Middleware::Next:
      return false;
    case Middleware::Break:
      res.status(404).send();
      return true;
    case Middleware::None:
      break;
    default:
      res.status(noMatch.val).send();
      return true;
    }
  }
  res.status(statusCode).send();
  return true;
}

std::ostream& HTTP::Routing::operator<<(std::ostream& os, const Module& module) {
  return os << "Routing::Module(" << Types::ToString(module.getType()) << ")";
}