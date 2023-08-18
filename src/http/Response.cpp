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
  req(&request),
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

Response& Response::operator=(const Response& other) {
  if (this == &other)
    return *this;
  this->req = other.req;
  this->headers = other.headers;
  this->body = other.body;
  this->statusCode = other.statusCode;
  this->statusMessage = other.statusMessage;
  this->sent = other.sent;
  this->route = other.route;
  this->init();
  return *this;
}

void Response::init() {
  this->headers.append("Server", settings->get<std::string>("misc.name"));
  this->headers.append("Date", Utils::getJSONDate());
  if (this->req->getHeaders().has("Connection"))
    this->headers.append("Connection", this->req->getHeaders().get<std::string>("Connection"));
  else if (!this->req->isExpecting() || this->req->getProtocol() == "HTTP/1.0")
    this->headers.append("Connection", "close");
}


Response& Response::setHeaders(const Headers& headers) {
  this->headers = headers;
  return *this;
}

Response& Response::setBody(const std::string& body) {
  this->body.clear();
  this->body.put(body);
  Logger::debug
    << "setting body to: <> of size: " << Logger::param(this->body.size())
    << "with str size: " << Logger::param(body.size())
    << std::endl;
  this->headers.set("Content-Length", this->body.size());
  return *this;
}

Response& Response::setBody(const ByteStream& body) {
  this->body = body;
  this->headers.set("Content-Length", this->body.size());
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

const std::string Response::getBody() const {
  return this->body.toString();
}

const ByteStream& Response::getRawBody() const {
  return this->body;
}

ByteStream& Response::getRawBody() {
  return this->body;
}


uint32_t Response::getStatus() const {
  return statusCode;
}

const std::string& Response::getStatusMessage() const {
  return statusMessage;
}

Response::operator std::string() {
  return this->toString<std::string>();
}

std::string Response::getHeader() const {
  std::stringstream ss;
  ss
    << this->req->getProtocol() << " "
    << this->statusCode << " "
    << this->statusMessage << "\r\n"
    << headers;

  return ss.str();
}

void Response::sendHeader() {
  const std::string header = this->getHeader() + "\r\n";
  Socket::Connection& client = const_cast<Request*>(this->req)->getClient();
  Logger::info
    << "Sending headers to: " << Logger::param(client) << std::endl
    << Logger::param(header) << std::endl;
  ByteStream& buffer = client.getWriteBuffer();
  buffer.put(header);
  this->afterSend();
}

void Response::send() {
  if (this->sent) return;
  if (this->route && this->route->hasErrorPage(this->statusCode) && this->body.empty()) {
    this->sendFile(this->route->getErrorPage(this->statusCode), false);
    return;
  }
  this->_preSend();
  const ByteStream resp = this->toString<ByteStream>();
  Socket::Connection& client = const_cast<Request*>(this->req)->getClient();
  Logger::info
    << "Sending response to: " << Logger::param(client) << std::endl
    << Logger::param(*this) << std::endl;
  ByteStream& buffer = client.getWriteBuffer();
  buffer.put(resp);
  this->afterSend();
}

void Response::redirect(const std::string& path, bool permanent) {
  this->status(permanent ? 301 : 307);
  this->headers.set("Location", Utils::encodeURIComponent(path));
  this->send();
}

void Response::_sendChunk(const char* buffer, std::istream& stream, bool last) {
  Socket::Connection& client = const_cast<Request*>(this->req)->getClient();

  std::stringstream chunk;
  const size_t chunkSize = stream.gcount();
  chunk << std::hex << chunkSize << "\r\n";
  chunk.write(buffer, chunkSize);
  chunk << "\r\n";
  if (last)
    chunk << "0\r\n\r\n";
  Logger::debug
    << "Sending chunk of size: " << Logger::param(chunkSize) << std::endl;
  // << Logger::param(chunk.str()) << std::endl;
  ByteStream& buff = client.getWriteBuffer();
  buff.put(chunk.str());
  if (last)
    this->afterSend();
}

void Response::_preSend() {
  if (!this->headers.has("Content-Length"))
    this->headers.set("Content-Length", this->body.size());
}

void Response::_preStream(const std::string& filePath) {
  this->headers.set("Transfer-Encoding", "chunked");
  this->headers.remove("Content-Length");
  const std::string ext = Utils::getExtension(filePath);
  this->headers.set("Content-Type", settings->httpMimeType(ext));
}

void Response::stream(std::istream& buff, size_t fileSize /* = 0 */) {
  if (this->req->getMethod() == Methods::HEAD)
    return;
  static const int nbrOfChunks = settings->get<int>("http.static.file_chunks");

  int n;
  if (fileSize == 0)
    n = settings->get<int>("http.static.file_chunk_size");
  else
    n = fileSize / nbrOfChunks;
  char buffer[n];
  while (buff.read(buffer, n))
    this->_sendChunk(buffer, buff);

  this->_sendChunk(buffer, buff, true);
}

void Response::sendFile(
  const std::string& filePath,
  bool stream /* = true */,
  struct stat* fileStat /* = NULL */
) {
  try {
    std::ifstream file(filePath.c_str());
    if (!file.is_open() || !file.good())
      throw std::runtime_error("Could not open file " + filePath);
    this->setupStaticFileHeaders(filePath, fileStat);
    if (stream) {
      this->_preStream(filePath);
      this->sendHeader();
      this->stream(reinterpret_cast<std::istream&>(file), fileStat ? fileStat->st_size : 0);
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
    const_cast<Request*>(this->req)->getClient().markToClose();
}

void Response::setupStaticFileHeaders(const std::string& filePath, struct stat* fileStat /* = NULL */) {
  bool noFileStat = fileStat == nullptr;
  if (fileStat == nullptr) {
    fileStat = new struct stat;
    if (stat(filePath.c_str(), fileStat) == -1)
      throw std::runtime_error("Could not stat file " + filePath);
  }
  this->headers.set("Last-Modified", Utils::getJSONDate(fileStat->st_mtime));
  this->headers.set("ETag", Utils::httpETag(filePath, fileStat->st_mtime, fileStat->st_size));
  if (noFileStat)
    delete fileStat;
}

std::ostream& HTTP::operator<<(std::ostream& os, const Response& res) {
  os << "Response(" << res.getStatus() << ", " << res.getStatusMessage() << "):" << std::endl
    << "Headers: " << std::endl << res.getHeaders() << std::endl;
  return os;
}