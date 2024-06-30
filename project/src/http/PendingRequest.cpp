#include <socket/Parallel.hpp>
#include "http/PendingRequest.hpp"
#include <utils/misc.hpp>

using namespace HTTP;

std::string PendingRequest::States::ToString(State state) {
  switch (state) {
  case States::Uri: return "Uri";
  case States::Body: return "Body";
  case States::Headers: return "Headers";
  case States::BodyChunkSize: return "BodyChunkSize";
  case States::BodyChunkData: return "BodyChunkData";
  case States::Done: return "Done";
  default:
    return "UNK" + Utils::toString(state);
  }
}

PendingRequest::PendingRequest(
  Socket::Parallel* server,
  Socket::Connection* client
) :
  Request(server, client, Methods::UNK, "", ByteStream(), "", Headers(), std::map<std::string, std::string>(), std::vector<File>()),
  state(States::Uri),
  buildingHeaderKey(""),
  chunkSize(0),
  chunkData() {}

PendingRequest::PendingRequest() {}
PendingRequest::~PendingRequest() {}

PendingRequest::PendingRequest(const PendingRequest& other) :
  Request(other),
  state(other.state),
  buildingHeaderKey(other.buildingHeaderKey) {}

PendingRequest& PendingRequest::operator=(const PendingRequest& other) {
  if (this == &other) return *this;
  this->Request::operator=(other);
  this->state = other.state;
  this->buildingHeaderKey = other.buildingHeaderKey;
  return *this;
}

PendingRequest::States::State PendingRequest::getState() const {
  return this->state;
}

void PendingRequest::setState(const States::State state) {
  this->state = state;
}

void PendingRequest::next() {
  States::State nextState = static_cast<States::State>(this->state + 1);
  Logger::debug
    << "PendingRequest::next(): "
    << Logger::param(States::ToString(this->state))
    << " -> "
    << Logger::param(States::ToString(nextState))
    << std::newl;
  this->state = nextState;
}

Headers& PendingRequest::getHeaders() {
  return this->headers;
}

void PendingRequest::setMethod(const Methods::Method method) {
  this->method = method;
}

void PendingRequest::setPath(const std::string& path) {
  this->path = Utils::resolvePath(1, path.c_str());
}

PendingRequest::Protocol PendingRequest::getProtocol() const {
  // extract protocol, versionMajor  and versionMinor
  std::string protocol = this->protocol;
  std::string versionMajor = "1";
  std::string versionMinor = "1";
  std::string::size_type pos = protocol.find("/");
  if (pos != std::string::npos) {
    versionMajor = protocol.substr(pos + 1);
    protocol = protocol.substr(0, pos);
  }
  pos = versionMajor.find(".");
  if (pos != std::string::npos) {
    versionMinor = versionMajor.substr(pos + 1);
    versionMajor = versionMajor.substr(0, pos);
  }
  return (Protocol) {
    protocol,
      Utils::to<int>(versionMajor),
      Utils::to<int>(versionMinor)
  };
}

void PendingRequest::setProtocol(const std::string& protocol) {
  this->protocol = protocol;
}

void PendingRequest::setProtocolType(const std::string& protocolType) {
  Protocol protocol = this->getProtocol();
  protocol.type = protocolType;
  this->protocol = protocol;
}

void PendingRequest::setVersionMajor(const int versionMajor) {
  Protocol protocol = this->getProtocol();
  protocol.versionMajor = versionMajor;
  this->protocol = protocol;
}

void PendingRequest::setVersionMinor(const int versionMinor) {
  Protocol protocol = this->getProtocol();
  protocol.versionMinor = versionMinor;
  this->protocol = protocol;
}

void PendingRequest::setBody(const std::string& body) {
  this->body.clear();
  this->body.put(body);
}

void PendingRequest::setBody(const ByteStream& body) {
  this->body = body;
}

void PendingRequest::addToBody(const std::string& body) {
  this->body.put(body);
}

void PendingRequest::addToBody(const ByteStream& body, size_t size) {
  if (size == static_cast<size_t>(-1))
    this->body.put(body);
  else
    this->body.put(body, size);
}

void PendingRequest::setHeaders(const Headers& headers) {
  this->headers = headers;
}

std::ostream& HTTP::operator<<(std::ostream& os, const PendingRequest& req) {
  os
    << "PendingRequest("
    << Methods::ToString(req.getMethod()) << "[" << req.getMethod() << "]"
    << ", "
    << req.getPath()
    << ", "
    << req.Request::getProtocol()
    << ") -> headers:\n"
    << req.getHeaders()
    << "\nPendingBodySize: " << Logger::param(req.chunkData.size())
    // << " -> body:\n"
    // << "---" << std::newl
    // << req.getRawBody()
    // << "---" << std::newl
    << std::newl;
  return os;
}

bool PendingRequest::lastCheck() {
  switch (this->getState()) {
  case States::Done:
    return true;
  case States::Body:
    if (this->chunkData.size() == this->getContentLength()) {
      this->setBody(this->chunkData);
      this->reset(true);
      return true;
    }
    return false;
  case States::BodyChunkSize:
  case States::BodyChunkData:
    return this->chunkSize == 0;
  default:
    return false;
  }
}