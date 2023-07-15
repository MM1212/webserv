#include "modules/Socket.hpp"

using Socket::Connection;

Connection::Connection(int clientId, Server* server) {
  this->clientId = clientId;
  this->server = server;
  // TODO: Get address and port from client
  const char* address = "";
  sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  getpeername(clientId, (sockaddr*)&clientAddress, &clientAddressLength);
  this->address = inet_ntoa(clientAddress.sin_addr);
  this->port = ntohs(clientAddress.sin_port);
}

inline const int Connection::getId() const {
  return this->clientId;
}

inline const Socket::Server* Connection::getServer() const {
  return this->server;
}

inline const std::string& Connection::getAddress() const {
  return this->address;
}

inline const int Connection::getPort() const {
  return this->port;
}

inline const bool Connection::isAlive() const {
  return poll(&(pollfd) { this->clientId, POLLIN, 0 }, 1, 0) > 0;
}

template <typename T>
const bool Connection::send(const T& data, const int flags, const int timeout) const {
  if (this->isAlive())
    return SOCK_SEND(this->clientId, &data, size, flags) > 0;
  return false;
}

const bool Connection::disconnect() {
  if (this->isAlive()) {
    close(this->clientId);
    return true;
  }
  return false;
}