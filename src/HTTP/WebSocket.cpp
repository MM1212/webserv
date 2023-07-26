#include "HTTP.hpp"

using namespace HTTP;

WebSocket::WebSocket(::Server* _server)
  : Socket::Server(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP),
  server(_server) {}

WebSocket::~WebSocket() {}

WebSocket::WebSocket(const WebSocket& other)
  : Socket::Server(other),
  server(other.server) {}

WebSocket& WebSocket::operator=(const WebSocket& other) {
  this->Socket::Server::operator=(other);
  if (this != &other)
    this->server = other.server;
  return *this;
}

void WebSocket::onStarted() {
  this->server->onStart();
}

void WebSocket::onNewConnection(Socket::Connection& connection) {
  this->server->onNewConnection(connection);
}

void WebSocket::onDisconnected(Socket::Connection& connection) {
  this->server->onDisconnected(connection);
}

void WebSocket::onData(Socket::Connection& connection, const std::string& buffer) {
  this->server->onData(connection, buffer);
}