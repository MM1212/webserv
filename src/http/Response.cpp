#include "http/Response.hpp"

#include <utils/misc.hpp>
#include <utils/Logger.hpp>

#include <Settings.hpp>

using HTTP::Request;
using HTTP::Headers;
using HTTP::Response;
using HTTP::Route;

static const Settings* settings = Instance::Get<Settings>();

Response::Response(const Request& request, const Route* route)
  :
  req(request),
  headers(),
  body(),
  statusCode(-1),
  statusMessage(),
  sent(),
  route(route) {
  this->init();
}

Response::~Response() {}

Response::Response(const Response& other)
  :
  req(other.req),
  headers(other.headers),
  body(other.body),
  statusCode(other.statusCode),
  statusMessage(other.statusMessage),
  sent(other.sent),
  route(other.route) {
  this->init();
}

void Response::init() {
  this->headers.append("Server", settings->get<std::string>("misc.name"));
  this->headers.append("Date", Utils::getJSONDate());
  if (this->req.getHeaders().has("Connection"))
    this->headers.append("Connection", this->req.getHeaders().get<std::string>("Connection"));
  else if (!this->req.isExpecting() || this->req.getProtocol() == "HTTP/1.0")
    this->headers.append("Connection", "close");
}


Response& Response::setHeaders(const Headers& headers) {
  this->headers = headers;
  return *this;
}

Response& Response::setBody(const std::string& body) {
  this->body = body;
  this->headers.set("Content-Length", this->body.length());
  return *this;
}

Response& Response::setRoute(const Route* route) {
  this->route = route;
  return *this;
}

Response& Response::status(uint32_t status) {
  this->statusCode = status;
  this->statusMessage = settings->httpStatusCode(status);
  return *this;
}

Response& Response::status(uint32_t status, const std::string& message) {
  this->statusCode = status;
  this->statusMessage = message;
  return *this;
}

Headers& Response::getHeaders() const {
  return const_cast<Response*>(this)->headers;
}

const std::string& Response::getBody() const {
  return this->body;
}

uint32_t Response::getStatus() const {
  return statusCode;
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
    << this->statusCode << " "
    << this->statusMessage << "\r\n"
    << headers;

  return ss.str();
}

std::string Response::toString() const {
  std::stringstream ss;
  ss << this->getHeader()
    << "\r\n";
  if (this->body.length() > 0 && this->req.getMethod() != Methods::HEAD)
    ss << this->body;
  return ss.str();
}

void Response::sendHeader() {
  const std::string header = this->getHeader() + "\r\n";
  Socket::Connection& client = const_cast<Request&>(this->req).getClient();
  Logger::info
    << "Sending headers to: " << Logger::param(client) << std::endl
    << Logger::param(header) << std::endl;
  client.getWriteBuffer().append(header);
  this->afterSend();
}

void Response::send() {
  if (this->sent) return;
  if (this->route && this->route->hasErrorPage(this->statusCode) && this->body.empty()) {
    this->sendFile(this->route->getErrorPage(this->statusCode), false);
    return;
  }
  this->_preSend();
  const std::string resp = this->toString();
  Socket::Connection& client = const_cast<Request&>(this->req).getClient();
  Logger::info
    << "Sending response to: " << Logger::param(client) << std::endl
    << Logger::param(*this) << std::endl;
  client.getWriteBuffer().append(resp);
  this->afterSend();
}

void Response::redirect(const std::string& path, bool permanent) {
  this->status(permanent ? 301 : 307);
  this->headers.set("Location", path);
  this->send();
}

void Response::_sendChunk(const char* buffer, std::istream& stream, bool last) {
  Socket::Connection& client = const_cast<Request&>(this->req).getClient();

  std::stringstream chunk;
  chunk << std::hex << stream.gcount() << "\r\n";
  chunk.write(buffer, stream.gcount());
  chunk << "\r\n";
  if (last)
    chunk << "0\r\n\r\n";
  Logger::debug
    << "Sending chunk:" << std::endl
    << Logger::param(chunk.str()) << std::endl;
  client.getWriteBuffer().append(chunk.str());
  if (last)
    this->afterSend();
}

void Response::_preSend() {
  if (!this->headers.has("Content-Length"))
    this->headers.set("Content-Length", this->body.length());
}

void Response::_preStream(const std::string& filePath) {
  this->headers.set("Transfer-Encoding", "chunked");
  this->headers.remove("Content-Length");
  const std::string ext = Utils::getExtension(filePath);
  this->headers.set("Content-Type", settings->httpMimeType(ext));
}

void Response::stream(std::istream& buff) {
  if (this->req.getMethod() == Methods::HEAD)
    return;

  const int n = 1024;
  char buffer[n];
  while (buff.read(buffer, n))
    this->_sendChunk(buffer, buff);

  this->_sendChunk(buffer, buff, true);
}

void Response::sendFile(const std::string& filePath, bool stream /* = true */) {
  try {
    std::ifstream file(filePath.c_str());
    if (!file.is_open() || !file.good())
      throw std::runtime_error("Could not open file " + filePath);
    if (stream) {
      this->_preStream(filePath);
      this->sendHeader();
      this->stream(reinterpret_cast<std::istream&>(file));
    }
    else {
      std::stringstream ss;
      ss << file.rdbuf();
      this->send(ss.str());
    }
    file.close();
  }
  catch (const std::exception& e) {
    Logger::error
      << "Could not send file " << Logger::param(filePath) << ": " << e.what() << std::endl;
    this->status(500).send();
  }
}

void Response::afterSend() {
  this->sent = true;
  if (this->headers.has("Connection") && this->headers.get<std::string>("Connection") == "close")
    const_cast<Request&>(this->req).getClient().markToClose();
}

std::ostream& HTTP::operator<<(std::ostream& os, const Response& res) {
  os << "Response(" << res.getStatus() << ", " << res.getStatusMessage() << "):" << std::endl
    << "Headers: " << std::endl << res.getHeaders() << std::endl;
  return os;
}