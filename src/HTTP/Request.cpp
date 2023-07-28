#include "HTTP.hpp"
#include <utils/misc.hpp>

using HTTP::Request;
using HTTP::Headers;
using HTTP::Methods;

Request::Request(
  Server* server,
  Socket::Connection* client,
  Methods::Method method,
  const std::string& path,
  const std::string& body,
  const std::string& protocol,
  const Headers& headers
) :
  headers(headers),
  method(method),
  path(path),
  body(body),
  protocol(protocol),
  server(server),
  client(client),
  params() {
  this->parseParams();
}

Request::Request() {}
Request::~Request() {}

Request::Request(const Request& other) :
  headers(other.headers),
  method(other.method),
  path(other.path),
  body(other.body),
  protocol(other.protocol),
  server(other.server),
  params(other.params) {}

Request& Request::operator=(const Request& other) {
  if (this == &other) return *this;
  this->headers = other.headers;
  this->method = other.method;
  this->path = other.path;
  this->body = other.body;
  this->protocol = other.protocol;
  this->server = other.server;
  this->params = other.params;
  return *this;
}

const Headers& Request::getHeaders() const {
  return this->headers;
}

Methods::Method Request::getMethod() const {
  return this->method;
}

const std::string& Request::getPath() const {
  return this->path;
}

const std::string& Request::getProtocol() const {
  return this->protocol;
}

const Socket::Connection& Request::getClient() const {
  return *this->client;
}

const std::map<std::string, std::string>& Request::getParams() const {
  return this->params;
}

void Request::parseParams() {
  std::string query;
  std::string::size_type pos = this->path.find('?');
  if (pos != std::string::npos) {
    query = this->path.substr(pos + 1);
    this->path = this->path.substr(0, pos);
  }
  std::vector<std::string> pairs = Utils::split(query, "&");
  for (std::vector<std::string>::iterator it = pairs.begin(); it != pairs.end(); it++) {
    std::vector<std::string> pair = Utils::split(*it, "=");
    if (pair.size() == 2)
      this->params[pair[0]] = pair[1];
  }
}

std::ostream& HTTP::operator<<(std::ostream& os, const Request& req) {
  os
    << "Request("
    << req.getMethod()
    << ", "
    << req.getPath()
    << ", "
    << req.getProtocol()
    << ") -> headers:\n"
    << req.getHeaders();
  return os;
}