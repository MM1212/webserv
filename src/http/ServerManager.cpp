#include "http/ServerManager.hpp"
#include "http/ServerConfiguration.hpp"
#include <csignal>

using namespace HTTP;

ServerManager::ServerManager() : WebSocket() {
  std::signal(SIGINT, ServerManager::onSIGINT);
}
ServerManager::~ServerManager() {
  for (size_t i = 0; i < this->servers.size(); i++) {
    delete this->servers[i];
  }
}

bool ServerManager::loadConfig(const std::string& path) {
  const_cast<YAML::Node&>(this->root) = YAML::LoadFile(path);
  try {
    if (!this->root.isValid() || !this->root.has("servers"))
      throw std::runtime_error("Invalid configuration file");
    if (!this->root["servers"].is<YAML::Types::Sequence>())
      throw std::runtime_error("servers isn't a sequence");
    for (size_t i = 0; i < this->root["servers"].size(); i++) {
      this->addServer(this->root["servers"][i]);
    };
    for (size_t i = 0; i < this->servers.size(); i++) {
      ServerConfiguration* server = this->servers[i];
      const std::vector<Socket::Host>& hosts = server->getHosts();
      for (uint64_t i = 0; i < hosts.size(); i++)
        if (!this->defaultServers.count(hosts[i]) || server->isDefaultHost())
          this->defaultServers[hosts[i]] = server;
    }
    // check for duplicated server names
    for (size_t i = 0; i < this->servers.size(); i++) {
      ServerConfiguration* server = this->servers[i];
      const std::vector<std::string>& names = server->getNames();
      for (size_t k = 0; k < this->servers.size(); k++) {
        // check if has colliding hosts
        const std::vector<Socket::Host>& hosts = this->servers[k]->getHosts();
        bool hasCollidingHost = false;
        for (size_t j = 0; j < hosts.size(); j++)
          if (i != k && server->hasHost(hosts[j])) {
            hasCollidingHost = true;
            break;
          }
        if (!hasCollidingHost)
          continue;
        for (size_t j = 0; j < names.size(); j++)
          if (i != k && this->servers[k]->hasName(names[j]))
            throw std::runtime_error("Duplicated server name: " + names[j]);
      }
    }
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load configuration file: "
      << Logger::param(e.what())
      << std::newl;
    return false;
  }
  return true;
}

void ServerManager::addServer(const YAML::Node& node) {
  ServerConfiguration* server = new ServerConfiguration(node);
  if (!server)
    throw std::runtime_error("Failed to create server");
  this->servers.push_back(server);
  server->init();
}

ServerConfiguration* ServerManager::selectServer(const Request& req) const {
  for (size_t i = 0; i < this->servers.size(); i++) {
    if (this->servers[i]->match(req))
      return this->servers[i];
  }
  return this->getDefaultServer(req.getServer());
}

ServerConfiguration* ServerManager::getDefaultServer(const Socket::Host host) const {
  if (this->defaultServers.count(host) > 0)
    return this->defaultServers.at(host);
  return nullptr;
}

void ServerManager::onRequest(const Request& req, Response& res) {
  ServerConfiguration* server = this->selectServer(req);
  if (!server) {
    res.status(501).send();
    return;
  }
  server->handleRequest(req, res);
}

void ServerManager::bindServers() {
  std::vector<Socket::ListenAddress> listenAddresses;
  for (size_t i = 0; i < this->servers.size(); i++) {
    const ServerConfiguration* server = this->servers[i];
    for (size_t j = 0; j < server->getHosts().size(); j++) {
      const Socket::Host& host = server->getHosts()[j];
      const Socket::ListenAddress addr(host.address, host.port, server->getMaxConnections());
      if (std::find(listenAddresses.begin(), listenAddresses.end(), addr) != listenAddresses.end())
        continue;
      listenAddresses.push_back(addr);
    }
  }
  Logger::debug
    << "Attempting to bind "
    << Logger::param(listenAddresses.size())
    << " addresses.." << std::newl;
  for (std::vector<Socket::ListenAddress>::iterator it = listenAddresses.begin(); it != listenAddresses.end(); it++) {
    const Socket::ListenAddress& address = *it;
    this->bind(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP, address, address.maxConnections);
    Logger::info << "Listening on " << Logger::param(address.address) << ":" << Logger::param(address.port) << std::newl;
  }
}

void ServerManager::onSIGINT(int signum) {
  static bool pressed = false;
  (void)signum;
  if (!pressed) {
    Logger::warning << "CTRL+C pressed, press again to exit" << std::newl;
    pressed = true;
    std::signal(SIGINT, ServerManager::onSIGINT);
    return;
  }
  Logger::info << "Shutting down.." << std::newl;
  Instance::Get<ServerManager>()->stop();
}

void ServerManager::setEnv(char** env) {
  this->env = env;
  uint64_t i;
  for (i = 0; env[i]; i++);
  this->envSize = i;
}