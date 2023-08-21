#include "http/ServerManager.hpp"
#include "http/ServerConfiguration.hpp"
#include <Yaml.hpp>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <Settings.hpp>

using namespace HTTP;

ServerConfiguration::ServerConfiguration(const YAML::Node& config)
  : config(config), hosts(), names(), defaultHost(false), defaultRoute(NULL), routes() {
  this->init();
}

ServerConfiguration::~ServerConfiguration() {
  if (this->defaultRoute)
    delete this->defaultRoute;
  for (std::map<std::string, const Route*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
    delete it->second;
}

ServerConfiguration::ServerConfiguration(const ServerConfiguration& other)
  : config(other.config),
  hosts(other.hosts),
  names(other.names),
  defaultHost(other.defaultHost),
  defaultRoute(other.defaultRoute) {
  for (std::map<std::string, const Route*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
    delete it->second;
  this->routes.clear();
  for (std::map<std::string, const Route*>::const_iterator it = other.routes.begin(); it != other.routes.end(); ++it) {
    this->routes[it->first] = new Route(*it->second);
    if (this->routes[it->first])
      const_cast<Route*>(this->routes[it->first])->init();
  }
  this->init();
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
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("max_connections"))
    return Instance::Get<Settings>()->get<int>("socket.max_connections");
  return settings["max_connections"].as<int>();
}

const Route* ServerConfiguration::getRoute(const std::string& path) const {
  if (this->routes.count(path) == 0)
    return NULL;
  return this->routes.at(path);
}

const Route* ServerConfiguration::getNearestRoute(const std::string& path) const {
  std::string currentPath = path;
  while (currentPath != "/" && currentPath.length() > 0) {
    if (this->routes.count(currentPath) == 0) {
      std::string dir = Utils::dirname(currentPath);
      if (dir == currentPath)
        break;
      currentPath = dir;
    }
    else
      return this->routes.at(currentPath);
  }
  if (this->getRoute(currentPath))
    return this->getRoute(currentPath);
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
  Logger::debug
    << "Handling request for " << Logger::param(req.getPath())
    << " with route " << Logger::param(*route)
    << std::endl;
  res.setRoute(route);
  if (!route->isMethodAllowed(req.getMethod()))
    return res.status(405).send();
  if (route->getMaxBodySize() > 0 && req.getContentLength() > route->getMaxBodySize())
    return res.status(413).send();
  try {
    route->handle(req, res);
  }
  catch (const std::exception& e) {
    Logger::error
      << "Could not handle request: " << e.what() << std::endl;
    res.status(500).send();
  }
}


void ServerConfiguration::validate() {
  if (this->hosts.empty())
    throw std::runtime_error("No hosts defined");
  if (this->names.empty())
    throw std::runtime_error("No names defined");
  if (!this->defaultRoute)
    throw std::runtime_error("Default Route could not be created");
}

void ServerConfiguration::init() {
  Logger::debug
    << "Initializing server configuration for "
    << Logger::param(this->config.toString()) << std::endl;
  if (!this->config.has("settings") || !this->config["settings"].is<YAML::Types::Map>())
    const_cast<YAML::Node&>(this->config)["settings"] = YAML::Node::NewMap("settings");
  if (this->config.has("listen"))
    this->initHosts();
  if (this->config.has("server_names"))
    this->initNames();
  if (this->config.has("default") && this->config["default"].as<bool>())
    this->defaultHost = true;
  this->defaultRoute = new Routes::Default(this->config, this);
  this->validate();
  if (this->config.has("routes"))
    this->initRoutes();
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
  const YAML::Node& names = this->config["server_names"];
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

void ServerConfiguration::initRoutes() {
  const YAML::Node& routes = this->config["routes"];
  if (!routes.is<YAML::Types::Sequence>())
    throw std::runtime_error("Routes must be a sequence");
  Logger::debug
    << "Initializing routes "
    << Logger::param(routes.toString()) << std::endl;
  for (
    YAML::Node::const_iterator it = routes.begin<YAML::Node::Sequence>();
    it != routes.end<YAML::Node::Sequence>();
    ++it
    ) {
    const YAML::Node& route = *it;
    if (!route.is<YAML::Types::Map>())
      throw std::runtime_error("Route must be a map");
    if (!route.has("uri") || route["uri"].getValue().empty())
      throw std::runtime_error("Route must have a valid uri");
    if (
      !route.has("modules") ||
      route["modules"].size() == 0 ||
      !route["modules"].is<YAML::Types::Sequence>()
      )
      throw std::runtime_error("Route must have modules");
    std::string& uri = const_cast<std::string&>(route["uri"].getValue());
    if (*uri.rbegin() == '/' && uri.size() > 1)
      uri.erase(uri.size() - 1, 1);
    Route* routeRef = new Route(this, route);
    if (routeRef)
      routeRef->init();
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
  Logger::info
    << "Added route "
    << Logger::param(path)
    << " to server "
    << Logger::param(this->getNames()[0])
    << std::endl;
}

std::ostream& HTTP::operator<<(std::ostream& os, const ServerConfiguration& server) {
  os << "ServerConfiguration("
    << "hosts: " << static_cast<std::string>(server.getHosts()[0])
    << ")";
  return os;
}