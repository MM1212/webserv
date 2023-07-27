#include "Socket.hpp"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <iomanip>

using Socket::Server;
using Socket::Connection;

std::ostream& operator<<(std::ostream& os, const Connection::IO& io) {
  return os <<
    "IO(read: " << io.read << ", "
    "write: " << io.write << ", "
    "error: " << io.error << ", "
    "raw: " << std::hex << io.raw << ")";
}

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
  connectionTimeout(0) {
  this->handleFd = socket(this->domain, this->type, this->protocol);
  if (this->handleFd < 0) {
    throw Errors::FailedToCreateSocket();
  }
  int opt = 1;
  setsockopt(this->handleFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  SYS_FNCTL(this->handleFd, F_SETFL, O_NONBLOCK);
}

Server::~Server() {
  std::cout << "closing server .." << std::endl;
  this->close();
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
  this->onStarted();
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
    this->onDisconnected(const_cast<Connection&>(connection));
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
    this->onNewConnection(const_cast<Connection&>(connection));
  }
}

void Server::pollData() {
  if (this->clients.empty())
    return;
  for (
    std::set<Connection>::iterator it = this->clients.begin();
    it != this->clients.end();
    ++it
    ) {
    Connection& connection = const_cast<Connection&>(*it);
    Connection::IO io = connection.poll(this->connectionTimeout);
    if (io.error) {
      this->disconnect(connection);
      continue;
    }
    if (!io.read)
      continue;
    std::cout << io << std::endl;
    char data[1024];
    int bytes = 1;
    while (bytes > 0) {
      bytes = recv(it->getId(), data, 1024, 0);
      if (bytes > 0)
      {
        data[bytes] = '\0';
        connection.buffer.append(data, bytes);
      }
    }
    if (bytes == 0) {
      this->disconnect(connection);
      continue;
    }
    else if (!connection.buffer.empty())
    {
      std::cout << "dispatching.." << std::endl;
      this->onData(connection, connection.buffer);
      connection.buffer.clear();
    }
  }
}
