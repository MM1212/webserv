#include "http/Route.hpp"
#include "http/ServerConfiguration.hpp"
#include <Settings.hpp>
#include <utils/Logger.hpp>

using HTTP::Route;

static const Settings* settings = Instance::Get<Settings>();

Route::Route(const ServerConfiguration* server, const YAML::Node& node) :
  server(server),
  node(node) {}

Route::~Route() {
  for (uint32_t i = 0; i < this->modules.size(); ++i)
    delete this->modules[i];
}

Route::Route(const Route& other) :
  server(other.server),
  node(other.node) {}

void Route::init(bool injectMethods /* = true */) {
  Logger::debug
    << "Initializing route: " << Logger::param(this->node) << std::endl;
  YAML::Node& root = const_cast<YAML::Node&>(this->node);
  if (!root.has("settings"))
    root.insert(YAML::Node::NewMap("settings"));
  YAML::Node& settings = root["settings"];
  if (!settings.has("methods")) {
    settings.insert(YAML::Node::NewSequence("methods"));
    if (injectMethods)
      settings["methods"].insert(YAML::Node::NewScalar("0", "GET"));
  }
  if (this->node.has("modules"))
    for (size_t i = 0; i < this->node["modules"].size(); i++)
      this->initModule(this->node["modules"][i]);
}

bool Route::hasErrorPage(int code) const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("error_pages") || !settings["error_pages"].is<YAML::Types::Map>()) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      return false;
    return defaultRoute->hasErrorPage(code);
  }
  return settings["error_pages"].has(Utils::to<std::string>(code));
}

const std::string& Route::getErrorPage(int code) const {
  const YAML::Node& settings = this->getSettings();
  if (!this->hasErrorPage(code))
    throw std::runtime_error("No error page for code " + Utils::to<std::string>(code));
  if (!settings.has("error_pages")) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      throw std::runtime_error("No error page for code " + Utils::to<std::string>(code));
    return defaultRoute->getErrorPage(code);
  }
  return settings["error_pages"][Utils::to<std::string>(code)].getValue();
}

int Route::getMaxBodySize() const {
  const YAML::Node& routeSettings = this->getSettings();
  if (!routeSettings.has("max_body_size")) {
    const Route* defaultRoute = this->server->getDefaultRoute();
    if (defaultRoute == this || !defaultRoute)
      return settings->get<int>("http.max_body_size");
    return defaultRoute->getMaxBodySize();
  }
  return routeSettings["max_body_size"].as<int>();
}

bool Route::isMethodAllowed(Methods::Method method) const {
  const YAML::Node& methods = this->getSettings()["methods"];
  const std::string methodStr = Methods::ToString(method);
  for (size_t i = 0; i < methods.size(); ++i) {
    if (methods[i].getValue() == methodStr)
      return true;
  }
  const Route* defaultRoute = this->server->getDefaultRoute();
  if (defaultRoute == this || !defaultRoute)
    return false;
  return defaultRoute->isMethodAllowed(method);
}

std::ostream& HTTP::operator<<(std::ostream& os, const Route& route) {
  os << "Route(" << route.getUri() << ")";
  return os;
}

void Route::handle(const Request& req, Response& res) const {
  for (size_t i = 0; i < this->modules.size(); ++i) {
    const Routing::Module* rModule = this->modules[i];
    if (req.isExpecting() && !rModule->supportsExpect())
      continue;
    Logger::debug
      << "is method " << Methods::ToString(req.getMethod())
      << " allowed for route " << Logger::param(Routing::Types::ToString(rModule->getType()))
      << "?" << std::boolalpha << rModule->isMethodAllowed(req.getMethod()) << std::endl;
    if (!rModule->isMethodAllowed(req.getMethod()))
      continue;
    if (rModule->handle(req, res))
      return;
  }
  res.status(501).send();
}

HTTP::Routing::Module* Route::getModule(const Routing::Types::Type type) const {
  for (size_t i = 0; i < this->modules.size(); ++i) {
    if (this->modules[i]->getType() == type)
      return this->modules[i];
  }
  return NULL;
}

void Route::addModule(Routing::Module* module) {
  this->modules.push_back(module);
  Logger::debug
    << "Added module: " << Logger::param(*module)
    << " to route: " << Logger::param(*this) << std::endl;
}

void Route::initModule(const YAML::Node& node) {
  if (!node.has("type"))
    throw std::runtime_error("Module type is required");
  const std::string& type = node["type"].getValue();
  Routing::Module* mod = Routing::Module::Create(type, *this, node);
  if (!mod)
    throw std::runtime_error("Unknown module type: " + type);
  this->addModule(mod);
}