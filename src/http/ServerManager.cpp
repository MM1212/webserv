#include "http/ServerManager.hpp"
#include "http/ServerConfiguration.hpp"

using namespace HTTP;

ServerManager::ServerManager() : WebSocket() {}
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
    bool hasDefault = false;
    for (size_t i = 0; i < this->servers.size(); i++) {
      if (this->servers[i]->isDefaultHost()) {
        if (hasDefault)
          throw std::runtime_error("Multiple default servers");
        hasDefault = true;
        this->defaultServer = this->servers[i];
        break;
      }
    }
    if (!hasDefault && this->servers.size() > 0)
      this->defaultServer = this->servers[0];
    else if (!hasDefault)
      throw std::runtime_error("No default server");
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load configuration file: "
      << Logger::param(e.what())
      << std::endl;
    return false;
  }
  return true;
}

void ServerManager::addServer(const YAML::Node& node) {
  ServerConfiguration* server = new ServerConfiguration(node);
  if (!server)
    throw std::runtime_error("Failed to create server");
  this->servers.push_back(server);
}

ServerConfiguration* ServerManager::selectServer(const Request& req) const {
  for (size_t i = 0; i < this->servers.size(); i++) {
    if (this->servers[i]->match(req))
      return this->servers[i];
  }
  return this->getDefaultServer();
}

ServerConfiguration* ServerManager::getDefaultServer() const {
  for (size_t i = 0; i < this->servers.size(); i++) {
    if (this->servers[i]->isDefaultHost())
      return this->servers[i];
  }
  return NULL;
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
    << " addresses.." << std::endl;
  for (std::vector<Socket::ListenAddress>::iterator it = listenAddresses.begin(); it != listenAddresses.end(); it++) {
    const Socket::ListenAddress& address = *it;
    this->bind(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP, address.address, address.port, address.maxConnections);
    Logger::info << "Listening on " << Logger::param(address.address) << ":" << Logger::param(address.port) << std::endl;
  }
}