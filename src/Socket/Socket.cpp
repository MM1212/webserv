#include "Socket.hpp"
#include "stdio.h"
#include <errno.h>
#include <string.h>

using Socket::Server;
using Socket::Connection;

Server::Server(
  const Socket::Domain::Handle domain,
  const Socket::Type::Handle type = Socket::Type::TCP,
  const Socket::Protocol::Handle protocol = Socket::Protocol::IP
) :
  domain(domain),
  type(type),
  protocol(protocol),
  handleFd(-1),
  port(0),
  address(""),
  clients(),
  connectionTimeout(0),
  dispatcher() {
  this->handleFd = socket(this->domain, this->type, this->protocol);
  if (this->handleFd < 0) {
    throw Errors::FailedToCreateSocket();
  }
  setsockopt(this->handleFd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
  SYS_FNCTL(this->handleFd, F_SETFL, O_NONBLOCK);
}

Server::~Server() {
  std::cout << "closing server .." << std::endl;
  this->close();
}

inline Socket::Domain::Handle Server::getDomain() const {
  return this->domain;
}

inline Socket::Type::Handle Server::getType() const {
  return this->type;
}

inline Socket::Protocol::Handle Server::getProtocol() const {
  return this->protocol;
}

inline const std::string& Server::getAddress() const {
  return this->address;
}

int Server::getPort() const {
  return this->port;
}

bool Server::listen(
  const std::string& address,
  const int port,
  const int backlog,
  const int timeout
) {
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
  if (SYS_LISTEN(this->handleFd, backlog) < 0) {
    return false;
  }
  this->dispatcher.dispatchEvent(Dispatch::StartedEvent(*this));
  while (1) {
    this->pollNewConnections();
    this->pollData();
  }
  perror("crashed");
  return true;
}

bool Server::isListening() const {
  if (this->handleFd < 0)
    return false;
  pollfd pfd = { this->handleFd, POLLERR, 0 };
  poll(&pfd, 1, 0);
  return !(pfd.revents & POLLERR);
}

bool Server::disconnect(const int clientId) {
  return this->disconnect(Connection(clientId, this));
}

bool Server::disconnect(const Connection& connection) {
  if (connection.disconnect()) {
    this->dispatcher.dispatchEvent(Dispatch::DisconnectedEvent(connection));
    return this->clients.erase(connection) > 0;
  }
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
    return SYS_CLOSE(this->handleFd) > 0;
  }
  return false;
}

Connection Server::acceptNewConnection() {
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);

  int clientId = SYS_ACCEPT(
    this->handleFd,
    (struct sockaddr*)&clientAddress,
    &clientAddressLength
  );
  if (clientId < 0) {
    return Connection(-1, this);
  }
  SYS_FNCTL(clientId, F_SETFL, O_NONBLOCK);
  return Connection(clientId, this);
}

void Server::pollNewConnections() {
  Connection connection = this->acceptNewConnection();
  if (connection.getId() > 0 && connection.isAlive()) {
    this->clients.insert(connection);
    this->dispatcher.dispatchEvent(Dispatch::NewConnectionEvent(connection));
  }
}

void Server::pollData() {
  if (this->clients.empty())
    return;
  // fd_set readFds, writeFds, errorFds;
  // FD_ZERO(&readFds);
  // FD_ZERO(&writeFds);
  // FD_ZERO(&errorFds);
  // int maxFd = this->clients.empty() ? this->handleFd : (this->clients.end()--)->getId();
  // for (
  //   std::set<Connection>::iterator it = this->clients.begin();
  //   it != this->clients.end();
  //   ++it
  //   ) {
  //   FD_SET(it->getId(), &readFds);
  //   FD_SET(it->getId(), &writeFds);
  //   FD_SET(it->getId(), &errorFds);
  // }
  // struct timeval waitd = {1, 0}; 
  // select(maxFd + 1, &readFds, &writeFds, &errorFds, &waitd);
  for (
    std::set<Connection>::iterator it = this->clients.begin();
    it != this->clients.end();
    ++it
    ) {
    Connection& connection = const_cast<Connection&>(*it);
    // if (FD_ISSET(it->getId(), &errorFds)) {
    //   std::cout << "error on connection " << it->getId() << std::endl;
    //   this->disconnect(connection);
    //   continue;
    // }
    // if (FD_ISSET(it->getId(), &readFds)) {
    //   std::cout << "reading data from " << it->getId() << std::endl;
    //   if (this->connectionTimeout > 0) {
    //     connection.ping();
    //   }
    char data[6];
    bzero(data, sizeof(data));

    int bytes = recv(it->getId(), data, 5, 0);
    if (bytes > 0)
      connection.buffer.append(data, bytes);
    else if (bytes == 0) {
      this->disconnect(connection);
      continue;
    }
    else if (!connection.buffer.empty())
    {
      this->dispatcher.dispatchEvent(Dispatch::DataEvent<std::string>(connection, connection.buffer));
      connection.buffer.clear();
    }
    // }
  }
}
