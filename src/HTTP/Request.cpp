#include "HTTP.hpp"
#include <utils/misc.hpp>

using HTTP::Request;
using HTTP::Headers;
using HTTP::Methods;

Request::Request(
  Server* server,
  Methods::Method method,
  const std::string& path,
  const std::string& body,
  const std::string& protocol,
  const Headers& headers,
  const std::map<std::string, std::string>& params
) :
  headers(headers),
  method(method),
  path(path),
  body(body),
  protocol(protocol),
  server(server),
  params(params) {
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
  headers = other.headers;
  method = other.method;
  path = other.path;
  body = other.body;
  protocol = other.protocol;
  server = other.server;
  params = other.params;
  return *this;
}

const Headers& Request::getHeaders() const {
  return headers;
}

Methods::Method Request::getMethod() const {
  return method;
}

const std::string& Request::getPath() const {
  return path;
}

const std::map<std::string, std::string>& Request::getParams() const {
  return params;
}

void Request::parseParams() {
  std::string query;
  std::string::size_type pos = path.find('?');
  if (pos != std::string::npos) {
    query = path.substr(pos + 1);
    path = path.substr(0, pos);
  }
  std::vector<std::string> pairs = Utils::split(query, "&");
  for (std::vector<std::string>::iterator it = pairs.begin(); it != pairs.end(); it++) {
    std::vector<std::string> pair = Utils::split(*it, "=");
    if (pair.size() == 2)
      params[pair[0]] = pair[1];
  }
}