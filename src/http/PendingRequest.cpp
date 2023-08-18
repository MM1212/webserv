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
  Request(server, client, Methods::UNK, "", ByteStream(), "", Headers(), std::map<std::string, std::string>(), std::vector<File>()),
  crlfNextState(States::Method),
  state(States::Method),
  buildingHeaderKey(""),
  chunkSize(0),
  chunkData() {}

PendingRequest::PendingRequest() {}
PendingRequest::~PendingRequest() {}

PendingRequest::PendingRequest(const PendingRequest& other) :
  Request(other),
  crlfNextState(other.crlfNextState),
  state(other.state),
  storage(other.storage),
  buildingHeaderKey(other.buildingHeaderKey) {}

PendingRequest& PendingRequest::operator=(const PendingRequest& other) {
  if (this == &other) return *this;
  this->Request::operator=(other);
  this->crlfNextState = other.crlfNextState;
  this->state = other.state;
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

void PendingRequest::nextWithCRLF(int nextState /* = -1 */) {
  this->crlfNextState = static_cast<States::State>(this->state + 1);
  if (nextState != -1)
    this->crlfNextState = static_cast<States::State>(nextState);
  Logger::debug
    << "PendingRequest::crlfCheck(): "
    << Logger::param(States::ToString(this->state))
    << " -> "
    << Logger::param(States::ToString(this->crlfNextState))
    << std::endl;
  this->state = States::CLRFCheck;
}

void PendingRequest::next() {
  States::State nextState = static_cast<States::State>(this->state + 1);
  if (this->state == States::CLRFCheck)
    nextState = this->crlfNextState;
  Logger::debug
    << "PendingRequest::next(): "
    << Logger::param(States::ToString(this->state))
    << " -> "
    << Logger::param(States::ToString(nextState))
    << std::endl;
  this->state = nextState;
}

Headers& PendingRequest::getHeaders() {
  return this->headers;
}

void PendingRequest::setMethod(const Methods::Method method) {
  this->method = method;
}

void PendingRequest::setPath(const std::string& path) {
  this->path = Utils::resolvePath(1, Utils::decodeURIComponent(path).c_str());
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

void PendingRequest::addToBody(const ByteStream& body) {
  this->body.put(body);
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
    // << " -> body:\n"
    // << "---" << std::endl
    // << req.getRawBody()
    // << "---" << std::endl
    << std::endl;
  return os;
}

void PendingRequest::handleMultiformData() {
  const Headers& headers = this->getHeaders();
  if (!headers.has("Content-Type") || headers.get<std::string>("Content-Type").compare("multipart/form-data") != 0)
    return;
  std::string boundary = headers.get<std::string>("Content-Type");
  boundary = boundary.substr(boundary.find("boundary=") + 9);
  std::string body = this->getRawBody();
  std::vector<std::string> parts = Utils::split(body, "--" + boundary);
  for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); it++) {
    std::string part = *it;
    if (part.empty()) continue;
    std::string::size_type pos = part.find("\r\n\r\n");
    if (pos == std::string::npos) continue;
    std::string headers = part.substr(0, pos);
    std::string data = part.substr(pos + 4);
    std::map<std::string, std::string> headersMap;
    std::vector<std::string> headersLines = Utils::split(headers, "\r\n");
    for (std::vector<std::string>::iterator it = headersLines.begin(); it != headersLines.end(); it++) {
      std::string header = *it;
      std::string::size_type pos = header.find(": ");
      if (pos == std::string::npos) continue;
      std::string key = header.substr(0, pos);
      std::string value = header.substr(pos + 2);
      headersMap[key] = value;
    }
    if (headersMap.find("Content-Disposition") == headersMap.end()) continue;
    std::string disposition = headersMap["Content-Disposition"];
    std::string::size_type cpos = disposition.find("name=\"");
    if (cpos == std::string::npos) continue;
    std::string name = disposition.substr(cpos + 6);
    cpos = name.find("\"");
    if (cpos == std::string::npos) continue;
    name = name.substr(0, cpos);
    if (headersMap.find("filename") != headersMap.end()) {
      std::string filename = headersMap["filename"];
      cpos = filename.find("\"");
      if (cpos == std::string::npos) continue;
      filename = filename.substr(0, cpos);
      this->files.push_back(File(name, filename, data));
    }
    else {
      this->params[name] = data;
    }
  }

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
  case States::BodyChunkBytes:
    return this->chunkSize == 0;
  default:
    return false;
  }
}