#include "socket/Connection.hpp"


using Socket::Connection;

Connection::Connection(
  File& handle,
  int sock,
  int timeout
)
  : handle(handle), serverSock(sock),
  timeout(timeout), heartbeat(Utils::getCurrentTime()),
  closeOnEmptyWriteBuffer(false) {
  this->init();
}

Connection::Connection(const Connection& other)
  : handle(other.handle), serverSock(other.serverSock),
  timeout(other.timeout), heartbeat(other.heartbeat),
  closeOnEmptyWriteBuffer(false) {
  this->init();
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

ByteStream& Connection::getReadBuffer() {
  return this->readBuffer;
}

ByteStream& Connection::getWriteBuffer() {
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

void Connection::disconnect() {
  if (!this->isAlive()) return;
  close(this->handle);
}