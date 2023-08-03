#include "socket/Connection.hpp"


using Socket::Connection;

Connection::Connection(
  File& handle,
  int sock,
  int timeout
)
  : handle(handle), serverSock(sock),
  timeout(timeout), heartbeat(Utils::getCurrentTime()) {
  this->init();
}

Connection::Connection(const Connection& other)
  : handle(other.handle), serverSock(other.serverSock),
  timeout(other.timeout), heartbeat(other.heartbeat) {
  this->init();
}

Connection& Connection::operator=(const Connection& other) {
  if (this == &other) return *this;
  this->handle = other.handle;
  this->serverSock = other.serverSock;
  this->timeout = other.timeout;
  this->heartbeat = other.heartbeat;
  this->init();
  return *this;
}

Connection::~Connection() {}

void Connection::init() {
  sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  getpeername(this->handle, (sockaddr*)&clientAddress, &clientAddressLength);
  this->address = inet_ntoa(clientAddress.sin_addr);
  this->port = ntohs(clientAddress.sin_port);
}

int Connection::getServerSock() const {
  return this->serverSock;
}

const Socket::File& Connection::getHandle() const {
  return this->handle;
}

const std::string& Connection::getAddress() const {
  return this->address;
}

int Connection::getPort() const {
  return this->port;
}

std::string Connection::getIpAddress() const {
  return this->address + ":" + Utils::toString(this->port);
}

std::stringstream& Connection::getReadBuffer() {
  return this->readBuffer;
}

std::string& Connection::getWriteBuffer() {
  return this->writeBuffer;
}

int Connection::getTimeout() const {
  return this->timeout;
}

uint64_t Connection::getHeartbeat() const {
  return this->heartbeat;
}

bool Connection::isAlive() const {
  return !this->handle.isClosed() && !this->handle.isErrored();
}

bool Connection::isReadable() const {
  return this->handle.isReadable();
}

bool Connection::isWritable() const {
  return this->handle.isWritable();
}

bool Connection::hasTimedOut() const {
  return Utils::getCurrentTime() - this->heartbeat >= static_cast<uint32_t>(this->timeout);
}

void Connection::ping() {
  this->heartbeat = Utils::getCurrentTime();
}

void Connection::send(const std::string& message) {
  if (!this->isWritable() || !this->isAlive() || this->hasTimedOut()) return;
  if (write(this->handle, message.c_str(), message.size()) >= 0)
    this->ping();
}

void Connection::read() {
  if (!this->isReadable() || !this->isAlive() || this->hasTimedOut()) return;
  char buffer[1025];
  int bytesRead = ::read(this->handle, buffer, 1024);
  buffer[bytesRead] = '\0';
  this->readBuffer << buffer;
  this->ping();
}

void Connection::disconnect() {
  if (!this->isAlive()) return;
  close(this->handle);
}