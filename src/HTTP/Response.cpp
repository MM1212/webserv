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
  this->headers.append("Content-Length", "0");
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
    << this->body
    << "\r\n";
  return ss.str();
}

void Response::send() {
  this->req.server->log.debug("Sending response: \n%d\n", this->sent);
  if (this->sent) return;
  const std::string resp = this->toString();
  this->req.server->log.debug("Sending response: \n%s\n", resp.c_str());
  if (this->req.getClient().send(resp.c_str(), resp.length(), 0, 0))
    this->sent = true;
}

