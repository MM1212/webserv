#include "HTTP.hpp"
#include <utils/misc.hpp>

using HTTP::Request;
using HTTP::Headers;
using HTTP::Methods;
using HTTP::Response;

Response::Response(const Request& request)
  :
  req(request),
  headers(),
  body(),
  status(-1),
  statusMessage() {
  this->init();
}

Response::~Response() {}

Response::Response(const Response& other)
  :
  req(other.req),
  headers(other.headers),
  body(other.body),
  status(other.status),
  statusMessage(other.statusMessage) {
  this->init();
}

void Response::init() {
  this->headers.append("Server", "webserv/1.0");
  this->headers.append("Date", Utils::getJSONDate());
  if (this->req.getHeaders().has("Connection"))
    this->headers.append("Connection", this->req.getHeaders().get("Connection"));
  else
    this->headers.append("Connection", "close");
}


Response& Response::setHeaders(const Headers& headers) {
  this->headers = headers;
  return *this;
}

Response& Response::setBody(const std::string& body) {
  this->body = body;
  this->headers.set("Content-Length", Utils::toString(this->body.length()));
  return *this;
}

Response& Response::setStatus(uint32_t status) {
  this->status = status;
  return *this;
}

Response& Response::setStatus(uint32_t status, const std::string& message) {
  this->status = status;
  this->statusMessage = message;
  return *this;
}

const Headers& Response::getHeaders() const {
  return headers;
}

const std::string& Response::getBody() const {
  return body;
}

uint32_t Response::getStatus() const {
  return status;
}

const std::string& Response::getStatusMessage() const {
  return statusMessage;
}

Response::operator std::string() {
  return this->toString();
}

std::string Response::getHeader() const {
  std::stringstream ss;
  ss
    << req.getProtocol() << " "
    << this->status << " "
    << this->statusMessage << "\r\n"
    << headers;

  return ss.str();
}

std::string Response::toString() const {
  std::stringstream ss;
  ss << this->getHeader()
    << "\r\n"
    << this->body;
  return ss.str();
}

bool Response::sendHead() {
  if (this->headersSent) return true;
  this->headersSent = true;
  const std::string header = this->getHeader();
  return this->req.getClient().send(header.c_str(), header.length(), 0, 0);
}

void Response::sendBody() {
  if (!this->headersSent) return this->send();
  this->req.getClient().send(this->body.c_str(), this->body.length(), 0, 0);
}

void Response::send() {
  if (!this->sendHead()) return;
  this->sendBody();
}

