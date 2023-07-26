#include "HTTP.hpp"
#include <utils/misc.hpp>

using HTTP::Server;
using HTTP::Route;
using HTTP::Methods;
using HTTP::Request;
using HTTP::Response;
using HTTP::Headers;

Server::Server()
  :
  router(),
  socket(this),
  log("Webserv") {
  this->loadStatusCodes();
}

Server::Server(const Server& other)
  :
  router(),
  socket(other.socket),
  log(other.log) {
  this->loadStatusCodes();
}

Server::~Server() {}

Server& Server::operator=(const Server& other) {
  this->socket = other.socket;
  this->router = other.router;
  this->log = other.log;
  return *this;
}

bool Server::listen(
  const std::string& address,
  const int port
) {
  return this->socket.listen(address, port, 128, 500);
}

void Server::onStart() {
  std::cout << "Listening on " << this->socket.getAddress() << ":" << this->socket.getPort() << std::endl;
}

void Server::onNewConnection(Socket::Connection& connection) {
  std::cout << "New connection from " << connection.getAddress() << ":" << connection.getPort() << std::endl;
}

void Server::onDisconnected(Socket::Connection& connection) {
  std::cout << "Disconnected from " << connection.getAddress() << ":" << connection.getPort() << std::endl;
}

void Server::onData(Socket::Connection& connection, const std::string& buffer) {
  std::vector<std::string> head_body = Utils::split(buffer, "\r\n\r\n");
  if (head_body.size() < 1) {
    this->log.error("Invalid request");
    connection.disconnect();
    return;
  }
  std::vector<std::string> head = Utils::split(head_body[0], "\r\n");
  if (head.size() < 1) {
    this->log.error("Invalid head size");
    connection.disconnect();
    return;
  }
  std::string first_line = head[0];
  head.erase(head.begin());
  std::vector<std::string> first_line_parts = Utils::split(first_line, " ");
  if (first_line_parts.size() < 3) {
    this->log.error("Invalid first line size");
    connection.disconnect();
    return;
  }
  Headers headers;
  for (std::vector<std::string>::iterator it = head.begin(); it != head.end(); ++it) {
    size_t pos = it->find_first_of(": ");
    if (pos == std::string::npos) {
      this->log.error("Invalid header format in %s", it->c_str());
      connection.disconnect();
      return;
    }
    std::string key = it->substr(0, pos);
    std::string value = it->substr(pos + 2);
    headers.append(key, value);
  }
  Request req(
    const_cast<Server*>(this),
    const_cast<Socket::Connection*>(&connection),
    Methods::fromString(first_line_parts[0]),
    first_line_parts[1],
    head_body[1],
    first_line_parts[2],
    headers
  );
  Response res(req);

  this->router.run(req, res);
}

void Server::loadStatusCodes() {
  try {
    this->statusCodes = YAML::LoadFile("config/bin/status.yaml");
    if (!this->statusCodes.is<YAML::Types::Map>())
      throw std::runtime_error("Expected a Map, got: " + this->statusCodes.toString());
  }
  catch (const std::exception& e) {
    throw std::runtime_error("Failed to load HTTP status codes: " + std::string(e.what()));
  }
}