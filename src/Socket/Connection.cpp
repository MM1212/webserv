#include "Socket.hpp"

using Socket::Connection;

Connection::Connection(int clientId, Server* server)
  : clientId(clientId), server(server), address(""), port(0), timeout(0), heartbeat(0), buffer() {
  // TODO: Get address and port from client
  sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  getpeername(clientId, (sockaddr*)&clientAddress, &clientAddressLength);
  this->address = inet_ntoa(clientAddress.sin_addr);
  this->port = ntohs(clientAddress.sin_port);
}

int Connection::getId() const {
  return this->clientId;
}

const Socket::Server* Connection::getServer() const {
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
  poll(&pfd, 1, 0);
  return !(pfd.revents & POLLERR);
}

template <typename T>
bool Connection::send(const T& data, const int flags, const int timeout) const {
  (void)timeout;
  if (this->isAlive())
    return SYS_SEND(this->clientId, &data, sizeof(T), flags) > 0;
  return false;
}

bool Connection::disconnect() const {
  if (this->isAlive()) {
    SYS_CLOSE(this->clientId);
    return true;
  }
  return false;
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