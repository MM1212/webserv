#include "Socket.hpp"

using namespace Socket;

ParallelServer::ParallelServer() : servers(), running(false) {}
ParallelServer::~ParallelServer() {
  this->stop();
  for (
    std::vector<Server*>::iterator it = this->servers.begin();
    it != this->servers.end();
    ++it
    ) {
    delete* it;
  }
}

ParallelServer::ParallelServer(const ParallelServer& other) {
  *this = other;
}

ParallelServer& ParallelServer::operator=(const ParallelServer& other) {
  this->servers = other.servers;
  this->running = other.running;
  return *this;
}

bool ParallelServer::add(Server* server, const std::string& address, const int port, const int backlog) {
  if (!server->bind(address, port, backlog))
    return false;
  if (this->running)
    server->onStarted();
  this->servers.push_back(server);
  return true;
}

void ParallelServer::start(const int timeout) {
  if (this->running) return;
  this->running = true;
  for (
    std::vector<Server*>::iterator it = this->servers.begin();
    it != this->servers.end();
    ++it
    ) {
    (*it)->connectionTimeout = timeout;
    (*it)->onStarted();
  }
  while (this->running) {
    for (
      std::vector<Server*>::iterator it = this->servers.begin();
      it != this->servers.end();
      ++it
      ) {
      (*it)->pollNewConnections();
      (*it)->pollData();
    }
  }
}

void ParallelServer::stop() {
  if (!this->running) return;
  this->running = false;
  for (
    std::vector<Server*>::iterator it = this->servers.begin();
    it != this->servers.end();
    ++it
    ) {
    (*it)->disconnectAll();
  }
}