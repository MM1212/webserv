#include "http/ServerManager.hpp"
#include "http/RouteStorage.hpp"
#include "http/ServerConfiguration.hpp"
#include <utils/misc.hpp>
#include <utils/Logger.hpp>

using namespace HTTP;

ServerConfiguration::ServerConfiguration(const YAML::Node& config)
  : config(config), hosts(), names(), defaultHost(false), defaultRoute(NULL), routes() {
  this->init();
}

ServerConfiguration::~ServerConfiguration() {
  for (std::map<std::string, const Route*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
    delete it->second;
}

ServerConfiguration::ServerConfiguration(const ServerConfiguration& other)
  : config(other.config) {
  *this = other;
}

ServerConfiguration& ServerConfiguration::operator=(const ServerConfiguration& other) {
  if (this == &other) return *this;

  this->hosts = other.hosts;
  this->names = other.names;
  this->defaultHost = other.defaultHost;
  if (this->defaultRoute)
    delete this->defaultRoute;
  this->defaultRoute = other.defaultRoute->clone();
  for (std::map<std::string, const Route*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
    delete it->second;
  this->routes = other.routes;
  for (std::map<std::string, const Route*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
    it->second = it->second->clone();
  this->init();
  return *this;
}

bool ServerConfiguration::match(const Request& req) const {
  ServerManager* serverManager = Instance::Get<ServerManager>();
  Socket::Server& server = serverManager->getServer(req.getClient().getServerSock());
  if (this->hasHost(Socket::Host(server.port, server.address)))
    return false;
  if (this->hasName(req.getHost()))
    return false;
  return true;
}

const std::vector<Socket::Host>& ServerConfiguration::getHosts() const {
  return this->hosts;
}

const std::vector<std::string>& ServerConfiguration::getNames() const {
  return this->names;
}

bool ServerConfiguration::hasName(const std::string& name) const {
  return std::find(this->names.begin(), this->names.end(), name) != this->names.end();
}

bool ServerConfiguration::hasHost(const Socket::Host& host) const {
  return std::find(this->hosts.begin(), this->hosts.end(), host) != this->hosts.end();
}

bool ServerConfiguration::isDefaultHost() const {
  return this->defaultHost;
}

void ServerConfiguration::setAsDefaultHost(bool state /* = true */) {
  this->defaultHost = state;
}

int ServerConfiguration::getMaxConnections() const {
  return this->config["max_connections"].as<int>();
}

const Route* ServerConfiguration::getRoute(const std::string& path) const {
  if (this->routes.count(path) == 0)
    return NULL;
  return this->routes.at(path);
}

const Route* ServerConfiguration::getNearestRoute(const std::string& path) const {
  std::string currentPath = path;
  while (currentPath != "/") {
    if (this->routes.count(currentPath) == 0)
      currentPath = Utils::dirname(currentPath);
    else
      return this->routes.at(currentPath);
  }
  return NULL;
}

const Routes::Default* ServerConfiguration::getDefaultRoute() const {
  return this->defaultRoute;
}

void ServerConfiguration::handleRequest(const Request& req, Response& res) const {
  res.setRoute(this->getDefaultRoute());
  const Route* route = this->getNearestRoute(req.getPath());
  if (!route)
    return res.status(404).send();
  bool isExact = route == this->getRoute(req.getPath());
  if (!isExact && !route->supportsCascade())
    return res.status(404).send();
  res.setRoute(route);
  if (!route->isMethodAllowed(req.getMethod()))
    return res.status(405).send();
  try {
    route->handle(req, res);
  }
  catch (const std::exception& e) {
    res.status(500).send();
    Logger::error
      << "Could not handle request: " << e.what() << std::endl;
  }
}


void ServerConfiguration::validate() {
  if (this->hosts.empty())
    throw std::runtime_error("No hosts defined");
  if (this->names.empty())
    throw std::runtime_error("No names defined");
}

void ServerConfiguration::init() {
  if (this->config.has("listen"))
    this->initHosts();
  if (this->config.has("names"))
    this->initNames();
  if (this->config.has("default") && this->config["default"].as<bool>())
    this->defaultHost = true;
  if (this->config.has("static") || this->config.has("cgi") || this->config.has("redirect"))
    this->initRootRoute();
  if (this->config.has("routes"))
    this->initRoutes();
  this->validate();
}

void ServerConfiguration::initHosts() {
  const YAML::Node& listen = this->config["listen"];
  switch (listen.getType()) {
  case YAML::Types::Scalar:
    this->parseHostNode(listen);
    break;
  case YAML::Types::Sequence:
    for (YAML::Node::const_iterator it = listen.begin<YAML::Node::Sequence>(); it != listen.end<YAML::Node::Sequence>(); ++it)
      this->parseHostNode(*it);
    break;
  default:
    throw std::runtime_error("Invalid listen value");
  }
}

void ServerConfiguration::parseHostNode(const YAML::Node& node) {
  const std::string& value = node.getValue();

  if (Utils::isInteger(value, true)) {
    const Socket::Host host(node.as<int>());
    if (this->hasHost(host))
      throw std::runtime_error("Duplicate host");
    this->hosts.push_back(host);
    return;
  }
  if (value.find(':') == std::string::npos)
    throw std::runtime_error("Invalid listen addr:port comb");
  std::vector<std::string> parts = Utils::split(value, ":");
  if (parts.size() != 2 || !Utils::isInteger(parts[1], true))
    throw std::runtime_error("Invalid listen addr:port comb");
  const Socket::Host host(std::atoi(parts[1].c_str()), parts[0]);
  if (this->hasHost(host))
    throw std::runtime_error("Duplicate host");
  this->hosts.push_back(host);
}

void ServerConfiguration::initNames() {
  const YAML::Node& names = this->config["names"];
  switch (names.getType()) {
  case YAML::Types::Scalar:
    if (this->hasName(names.getValue()))
      throw std::runtime_error("Duplicate name");
    this->names.push_back(names.getValue());
    break;
  case YAML::Types::Sequence:
    for (YAML::Node::const_iterator it = names.begin<YAML::Node::Sequence>(); it != names.end<YAML::Node::Sequence>(); ++it) {
      if (this->hasName(names.getValue()))
        throw std::runtime_error("Duplicate name");
      this->names.push_back(it->as<std::string>());
    }
    break;
  default:
    throw std::runtime_error("Invalid names value");
  }
}

void ServerConfiguration::initRootRoute() {
  this->defaultRoute = Instance::Get<RouteStorage>()->buildRoute<Routes::Default>(this->config, this);
}

void ServerConfiguration::initRoutes() {
  const YAML::Node& routes = this->config["routes"];
  if (!routes.is<YAML::Types::Sequence>())
    throw std::runtime_error("Routes must be a sequence");
  const RouteStorage* routeStorage = Instance::Get<RouteStorage>();
  for (
    YAML::Node::const_iterator it = routes.begin<YAML::Node::Sequence>();
    it != routes.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& route = *it;
    if (!route.is<YAML::Types::Map>())
      throw std::runtime_error("Route must be a map");
    if (!route.has("uri") || !route["uri"].is<std::string>())
      throw std::runtime_error("Route must have a uri");
    size_t presence = 0;
    if (route.has("static"))
      presence++;
    if (route.has("cgi"))
      presence++;
    if (route.has("redirect"))
      presence++;
    if (presence != 1)
      throw std::runtime_error("Route must have exactly one of static, cgi or redirect");
    const Route* routeRef = NULL;
    if (route.has("static"))
      routeRef = routeStorage->buildRoute<Routes::Static>(route, this);
    else if (route.has("redirect"))
      routeRef = routeStorage->buildRoute<Routes::Redirect>(route, this);
    else if (route.has("cgi"))
      routeRef = routeStorage->buildRoute<Routes::CGI>(route, this);
    if (!routeRef)
      throw std::runtime_error("Could not build route obj");
    try {
      this->addRoute(route["uri"].getValue(), routeRef);
    }
    catch (const std::exception& e) {
      delete routeRef;
      throw e;
    }
  }
}

void ServerConfiguration::addRoute(const std::string& path, const Route* route) {
  if (this->routes.count(path) > 0)
    throw std::runtime_error("Route " + path + "is already defined");
  this->routes.insert(
    std::make_pair(path, route)
  );
}