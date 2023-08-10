#include "http/Route.hpp"
#include "http/ServerConfiguration.hpp"
#include <Settings.hpp>

using HTTP::Route;

static const Settings* settings = Instance::Get<Settings>();

std::string Route::Types::ToString(Type type) {
  switch (type) {
  case Static: return "Static";
  case Redirect: return "Redirect";
  case CGI: return "CGI";
  default: return "Unknown";
  }
}

Route::Route(const ServerConfiguration* server, Types::Type type, const YAML::Node& node) :
  server(server),
  node(node),
  type(type) {
  this->init();
}

Route::~Route() {}

Route::Route(const Route& other) :
  server(other.server),
  node(other.node),
  type(other.type) {
  this->init();
}

void Route::init() {
  if (!this->node.has("uri") || this->node["uri"].getValue().empty())
    throw std::runtime_error("Uri is required for a route!");
  if (*this->node["uri"].getValue().rbegin() == '/')
    const_cast<std::string&>(this->node["uri"].getValue()).erase(this->node["uri"].getValue().size() - 1, 1);
  if (!this->node.has("methods")) {
    YAML::Node& root = const_cast<YAML::Node&>(this->node);
    root["methods"] = YAML::Node::NewSequence("methods");
    root["methods"].insert(YAML::Node::NewScalar("0", "GET"));
  }
  YAML::Node& methods = const_cast<YAML::Node&>(this->node["methods"]);
  bool hasGet = false;
  for (
    YAML::Node::const_iterator it = methods.begin<YAML::Node::Sequence>();
    it != methods.end<YAML::Node::Sequence>();
    ++it
    ) {
    if (it->getValue() == "GET") {
      hasGet = true;
      break;
    }
  }
  if (hasGet)
    methods.insert(YAML::Node::NewScalar("", "HEAD"));
}

Route::Types::Type Route::getType() const {
  return this->type;
}

const std::string& Route::getPath() const {
  return this->node["uri"].getValue();
}

bool Route::hasErrorPage(int code) const {
  if (!this->node.has("error_pages") || !this->node["error_pages"].is<YAML::Types::Map>()) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      return false;
    return defaultRoute->hasErrorPage(code);
  }
  return this->node["error_pages"].has(Utils::toString(code));
}

const std::string& Route::getErrorPage(int code) const {
  if (!this->hasErrorPage(code))
    throw std::runtime_error("No error page for code " + Utils::toString(code));
  if (!this->node.has("error_pages")) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      throw std::runtime_error("No error page for code " + Utils::toString(code));
    return defaultRoute->getErrorPage(code);
  }
  return this->node["error_pages"][Utils::toString(code)].getValue();
}

int Route::getMaxBodySize() const {
  if (!this->node.has("max_body_size")) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      return settings->get<int>("http.max_body_size");
    return defaultRoute->getMaxBodySize();
  }
  return this->node["max_body_size"].as<int>();
}

bool Route::isMethodAllowed(Methods::Method method) const {
  const YAML::Node& methods = this->node["methods"];
  const std::string methodStr = Methods::ToString(method);
  for (size_t i = 0; i < methods.size(); ++i) {
    if (methods[i].getValue() == methodStr)
      return true;
  }
  return false;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Route& route) {
  os << "Route(" << Route::Types::ToString(route.getType()) << ", " << route.getPath() << ")";
  return os;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Route* route) {
  os << "Route(" << Route::Types::ToString(route->getType()) << ", " << route->getPath() << ")";
  return os;
}