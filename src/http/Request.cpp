#include "http/Request.hpp"
#include <utils/misc.hpp>

using namespace HTTP;

Request::Request(
  Socket::Parallel* server,
  Socket::Connection* client,
  Methods::Method method,
  const std::string& path,
  const ByteStream& body,
  const std::string& protocol,
  const Headers& headers,
  const std::map<std::string, std::string>& params,
  const std::vector<File>& files
) :
  headers(headers),
  method(method),
  path(path),
  body(body),
  protocol(protocol),
  server(server),
  client(client),
  params(params),
  files(files) {
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
  client(other.client),
  params(other.params),
  files(other.files) {
  this->parseParams();
}

Request& Request::operator=(const Request& other) {
  if (this == &other) return *this;
  this->headers = other.headers;
  this->method = other.method;
  this->path = other.path;
  this->body = other.body;
  this->protocol = other.protocol;
  this->server = other.server;
  this->client = other.client;
  this->params = other.params;
  this->files = other.files;
  this->parseParams();
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

Socket::Connection& Request::getClient() {
  return *this->client;
}

const Socket::Server& Request::getServer() const {
  return this->server->getServer(this->client->getServerSock());
}

Socket::Server& Request::getServer() {
  return this->server->getServer(this->client->getServerSock());
}

const std::map<std::string, std::string>& Request::getParams() const {
  return this->params;
}

void Request::parseParams() {
  {
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
  {
    const Headers& headers = this->getHeaders();
    if (!headers.has("Content-Type") || headers.get<std::string>("Content-Type") != "application/x-www-form-urlencoded")
      return;
    std::vector<std::string> pairs = Utils::split(this->getBody(), "&");
    for (std::vector<std::string>::iterator it = pairs.begin(); it != pairs.end(); it++) {
      std::vector<std::string> pair = Utils::split(*it, "=");
      if (pair.size() == 2)
        this->params[pair[0]] = pair[1];
    }
    // this->body.clear();
  }
}

std::ostream& HTTP::operator<<(std::ostream& os, const Request& req) {
  os
    << "Request("
    << Methods::ToString(req.getMethod()) << "[" << req.getMethod() << "]"
    << ", "
    << req.getPath()
    << ", "
    << req.Request::getProtocol()
    << ", "
    << req.getQuery()
    << ") -> headers:\n"
    << req.getHeaders()
    // << " -> body:\n"
    // << "---" << std::newl
    // << req.getRawBody()
    // << "---" << std::newl
    << std::newl;
  return os;
}

const std::string Request::getUri() const {
  const std::string query = this->getQuery();
  if (query.empty())
    return this->path;
  return this->path + "?" + query;
}

const std::string Request::getQuery() const {
  std::string query;
  for (std::map<std::string, std::string>::const_iterator it = this->params.begin(); it != this->params.end(); it++) {
    if (!query.empty())
      query += "&";
    query += it->first + "=" + it->second;
  }
  return query;
}