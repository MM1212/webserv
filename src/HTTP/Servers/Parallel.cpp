#include "HTTP.hpp"

using namespace HTTP;

ParallelServer::ParallelServer() : sockets(), servers() {}
ParallelServer::~ParallelServer() {
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
  this->sockets = other.sockets;
  this->servers = other.servers;
  return *this;
}

bool ParallelServer::add(Server* server, const std::string& address, const int port, const int backlog) {
  WebSocket* socket = new WebSocket(server);
  if (!socket || !this->sockets.add(socket, address, port, backlog))
    return false;
  this->servers.push_back(server);
  return true;
}

void ParallelServer::start(const int timeout) {
  this->sockets.start(timeout);
}

void ParallelServer::stop() {
  this->sockets.stop();
}