#include <socket/Parallel.hpp>
#include "http/PendingRequest.hpp"
#include <utils/misc.hpp>

using namespace HTTP;

std::string PendingRequest::States::ToString(State state) {
  switch (state) {
  case States::CLRFCheck: return "CLRFCheck";
  case States::Method: return "Method";
  case States::Uri: return "Uri";
  case States::Protocol: return "Protocol";
  case States::VersionMajor: return "VersionMajor";
  case States::VersionMinor: return "VersionMinor";
  case States::Header: return "Header";
  case States::HeaderKey: return "HeaderKey";
  case States::HeaderValue: return "HeaderValue";
  case States::HeaderEnd: return "HeaderEnd";
  case States::Body: return "Body";
  case States::BodyChunked: return "BodyChunked";
  case States::BodyChunkBytes: return "BodyChunkBytes";
  case States::BodyChunkData: return "BodyChunkData";
  case States::BodyChunkEnd: return "BodyChunkEnd";
  case States::Done: return "Done";
  default:
    return "UNK" + Utils::toString(state);
  }
}

PendingRequest::PendingRequest(
  Socket::Parallel* server,
  Socket::Connection* client
) :
  crlfNextState(States::Method),
  state(States::Method),
  method(Methods::UNK),
  server(server),
  client(client),
  storage(""),
  buildingHeaderKey("") {}

PendingRequest::PendingRequest() {}
PendingRequest::~PendingRequest() {}

PendingRequest::PendingRequest(const PendingRequest& other) :
  crlfNextState(other.crlfNextState),
  state(other.state),
  headers(other.headers),
  method(other.method),
  path(other.path),
  body(other.body),
  protocol(other.protocol),
  server(other.server),
  params(other.params),
  storage(other.storage),
  buildingHeaderKey(other.buildingHeaderKey) {}

PendingRequest& PendingRequest::operator=(const PendingRequest& other) {
  if (this == &other) return *this;
  this->crlfNextState = other.crlfNextState;
  this->state = other.state;
  this->headers = other.headers;
  this->method = other.method;
  this->path = other.path;
  this->body = other.body;
  this->protocol = other.protocol;
  this->server = other.server;
  this->params = other.params;
  this->storage = other.storage;
  this->buildingHeaderKey = other.buildingHeaderKey;
  return *this;
}

PendingRequest::States::State PendingRequest::getState() const {
  return this->state;
}

void PendingRequest::setState(const States::State state) {
  this->state = state;
}

void PendingRequest::nextWithCRLF() {
  this->crlfNextState = this->state;
  this->state = States::CLRFCheck;
  Logger::debug
    << "PendingRequest::crlfCheck(): "
    << States::ToString(this->crlfNextState)
    << " -> "
    << States::ToString(static_cast<States::State>(this->crlfNextState + 1))
    << std::endl;
}

void PendingRequest::next() {
  if (this->state == States::CLRFCheck)
    this->state = this->crlfNextState;
  Logger::debug
    << "PendingRequest::next(): "
    << States::ToString(this->state)
    << " -> "
    << States::ToString(static_cast<States::State>(this->state + 1))
    << std::endl;
  this->state = static_cast<States::State>(this->state + 1);
}

Headers& PendingRequest::getHeaders() {
  return this->headers;
}

const Headers& PendingRequest::getHeaders() const {
  return this->headers;
}

Methods::Method PendingRequest::getMethod() const {
  return this->method;
}

const std::string& PendingRequest::getPath() const {
  return this->path;
}

const std::string& PendingRequest::getProtocol() const {
  return this->protocol;
}
const Socket::Connection& PendingRequest::getClient() const {
  return *this->client;
}

std::map<std::string, std::string>& PendingRequest::getParams() {
  return this->params;
}

void PendingRequest::setMethod(const Methods::Method method) {
  this->method = method;
}

void PendingRequest::setPath(const std::string& path) {
  this->path = path;
}

void PendingRequest::setProtocol(const std::string& protocol) {
  this->protocol = protocol;
}

void PendingRequest::setBody(const std::string& body) {
  this->body = body;
}

void PendingRequest::addToBody(const std::string& body) {
  this->body.append(body);
}

void PendingRequest::setHeaders(const Headers& headers) {
  this->headers = headers;
}

PendingRequest::operator Request() const {
  Request req(
    this->server,
    this->client,
    this->method,
    this->path,
    this->body,
    this->protocol,
    this->headers
  );
  return req;
}



void PendingRequest::parseParams() {
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

std::ostream& HTTP::operator<<(std::ostream& os, const PendingRequest& req) {
  os
    << "PendingRequest("
    << Methods::ToString(req.getMethod()) << "[" << req.getMethod() << "]"
    << ", "
    << req.getPath()
    << ", "
    << req.getProtocol()
    << ") -> headers:\n"
    << req.getHeaders()
    << " -> body:\n"
    << req.getBody<std::string>()
    << std::endl;
  return os;
}