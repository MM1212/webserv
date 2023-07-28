#include "HTTP.hpp"

using namespace HTTP;

SingleServer::SingleServer() : HTTP::Server(), socket(this) {}
SingleServer::~SingleServer() {}

SingleServer::SingleServer(const SingleServer& other) : HTTP::Server(other), socket(this) {
  *this = other;
}

SingleServer& SingleServer::operator=(const SingleServer& other) {
  this->socket = other.socket;
  return *this;
}

bool SingleServer::listen(
  const std::string& address,
  const int port
) {
  return this->socket.listen(address, port, 128, 500);
}

void SingleServer::onStart() {
  std::cout << "Listening on " << this->socket.getAddress() << ":" << this->socket.getPort() << std::endl;
}