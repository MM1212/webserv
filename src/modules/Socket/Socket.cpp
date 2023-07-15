#include "modules/Socket.hpp"

using Socket::Server;
using Socket::Connection;

Server::Server(
  const Socket::Domain::Handle domain,
  const Socket::Type::Handle type = Socket::Type::TCP,
  const Socket::Protocol::Handle protocol = Socket::Protocol::IP
) : domain(domain), type(type), protocol(protocol) {
  this->handleFd = socket(this->domain, this->type, this->protocol);
  if (this->handleFd < 0) {
    throw Errors::FailedToCreateSocket();
  }
}

Server::~Server() {
  this->close();
}

inline const Socket::Domain::Handle Server::getDomain() const {
  return this->domain;
}

inline const Socket::Type::Handle Server::getType() const {
  return this->type;
}

inline const Socket::Protocol::Handle Server::getProtocol() const {
  return this->protocol;
}

bool Server::listen(
  const std::string& address,
  const int port,
  const int backlog = 10,
  const int timeout = 0
) {
  if (this->isListening()) {
    return false;
  }
  this->address = address;
  this->port = port;
  this->connectionTimeout = timeout;
  sockaddr_in serverAddress;
  serverAddress.sin_family = this->domain;
  serverAddress.sin_addr.s_addr = inet_addr(this->address.c_str());
  serverAddress.sin_port = htons(this->port);
  if (bind(this->handleFd, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    return false;
  }
  if (SOCK_LISTEN(this->handleFd, backlog) < 0) {
    return false;
  }
  while (this->isListening()) {
    Connection connection = this->accept();
    if (connection.getId() > 0 && connection.isAlive())
      this->clients.insert(connection);
  }
}

bool Server::isListening() const {
  return this->handleFd > 0 && poll(&(pollfd) { this->handleFd, POLLIN, 0 }, 1, 0) > 0;
}

bool Server::disconnect(const int clientId) {
  return this->disconnect(Connection(clientId, this));
}

bool Server::disconnect(const Connection& connection) {
  if (connection.disconnect())
    return this->clients.erase(connection) > 0;
  return false;
}

bool Server::disconnectAll() {
  bool result = true;
  for (
    std::set<Connection>::iterator it = this->clients.begin();
    it != this->clients.end();
    ++it
    ) {
    result &= this->disconnect(*it);
  }
  return result;
}

bool Server::close() {
  if (this->isListening()) {
    this->disconnectAll();
    return SOCKET_CLOSE(this->handleFd) > 0;
  }
  return false;
}

Connection Server::accept() {
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);

  int clientId = SOCK_ACCEPT(
    this->handleFd,
    (struct sockaddr*)&clientAddress,
    &clientAddressLength
  );
  if (clientId < 0) {
    return Connection(-1, this);
  }
  return Connection(clientId, this);
}
