#include "Socket.hpp"

using Socket::Connection;

Connection::Connection(const int clientId, Server* server)
  : clientId(clientId), server(server), address(""), port(0), timeout(0), heartbeat(0), buffer() {
  // TODO: Get address and port from client
  sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  getpeername(clientId, (sockaddr*)&clientAddress, &clientAddressLength);
  this->address = inet_ntoa(clientAddress.sin_addr);
  this->port = ntohs(clientAddress.sin_port);
}

Connection::Connection(const Connection& other) {
  *this = other;
}

Connection& Connection::operator=(const Connection& other) {
  if (this == &other) return *this;
  this->clientId = other.clientId;
  this->server = other.server;
  this->address = other.address;
  this->port = other.port;
  this->timeout = other.timeout;
  this->heartbeat = other.heartbeat;
  return *this;
}

int Connection::getId() const {
  return this->clientId;
}

Socket::Server* Connection::getServer() const {
  return this->server;
}

const std::string& Connection::getAddress() const {
  return this->address;
}

int Connection::getPort() const {
  return this->port;
}

bool Connection::isAlive() const {
  pollfd pfd = { this->clientId, POLLERR, 0 };
  SYS_POLL(&pfd, 1, 0);
  return !(pfd.revents & POLLERR);
}

Connection::IO Connection::poll(const int timeout) const {
  pollfd pfd = { this->clientId, POLLIN | POLLOUT | POLLERR, 0 };
  SYS_POLL(&pfd, 1, timeout);
  return (Connection::IO){
    static_cast<bool>(pfd.revents & POLLIN),
    static_cast<bool>(pfd.revents & POLLOUT),
    static_cast<bool>(pfd.revents & POLLERR)
  };
}

bool Connection::send(const void* data, const uint32_t size, const int flags, const int timeout) const {
  (void)timeout;
  if (this->isAlive())
    return SYS_SEND(this->clientId, data, size, flags) > 0;
  return false;
}

bool Connection::disconnect() const {
  SYS_CLOSE(this->clientId);
  return true;
}

void Connection::ping() {
  if (this->isAlive()) {
    this->heartbeat = time(NULL);
  }
}

bool Connection::timedOut() const {
  const uint32_t now = time(NULL);
  return now - this->heartbeat > this->timeout;
}

Connection::operator int() const {
  return this->clientId;
}