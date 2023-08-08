#include "http/WebSocket.hpp"
#include <Settings.hpp>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <http/utils.hpp>

using namespace HTTP;

typedef HTTP::PendingRequest::States ReqStates;

static Settings* settings = Instance::Get<Settings>();

WebSocket::WebSocket()
  : Socket::Parallel(settings->get<int>("socket.keep_alive_timeout")) {}

WebSocket::~WebSocket() {}

void WebSocket::onClientConnect(const Socket::Connection& sock) {
  this->pendingRequests.insert(
    std::make_pair<int, PendingRequest>(
      sock,
      PendingRequest(
        this,
        const_cast<Socket::Connection*>(&sock)
      )
    )
  );
}

void WebSocket::onClientDisconnect(const Socket::Connection& sock) {
  this->pendingRequests.erase(sock);
}

void WebSocket::onClientRead(Socket::Connection& sock) {
  this->handleClientPacket(sock);
}

void WebSocket::handleClientPacket(Socket::Connection& sock) {
  if (!this->pendingRequests.count(sock)) {
    Logger::warning
      << "Received packet from unknown client " << Logger::param(sock)
      << std::endl;
    return;
  }
  PendingRequest& pendingRequest = this->pendingRequests.at(sock);
  std::stringstream packet;
  packet << sock.getReadBuffer().rdbuf();
  sock.getReadBuffer().str("");

  bool skip = false;

  while (skip || (packet.peek() != EOF && pendingRequest.getState() != ReqStates::Done)) {
    pendingRequest.storage += packet.get();
    if (skip)
      skip = false;
    /* if (std::isprint(packet.peek())) {
      Logger::debug
        << "peek: " << Logger::param<char>(packet.peek())
        << " | at: " << Logger::param(ReqStates::ToString(pendingRequest.getState()))
        << std::endl;
    }
    else {
      Logger::debug
        << "peek: " << std::hex << Logger::param(packet.peek()) << std::dec
        << " | at: " << Logger::param(ReqStates::ToString(pendingRequest.getState()))
        << std::endl;
    } */
    switch (pendingRequest.state) {
    case ReqStates::CLRFCheck:
    {
      if (pendingRequest.storage == std::crlf) {
        pendingRequest.storage.clear();
        pendingRequest.next();
        skip = true;
        continue;
      }
      else if (pendingRequest.storage.size() >= std::crlf.size()) {
        // send 400 (Bad Request)
        return this->sendBadRequest(sock, 400, "Invalid CRLF at " + ReqStates::ToString(pendingRequest.getState()));
      }
      break;
    }
    case ReqStates::Method:
    {
      if (packet.peek() == ' ') {
        packet.ignore();
        const std::string methodStr = pendingRequest.storage;
        pendingRequest.storage.clear();
        const Methods::Method method = Methods::FromString(methodStr);
        Logger::debug
          << "method: " << Logger::param(methodStr) << " (" << Logger::param(method) << ")"
          << std::endl;
        if (method == Methods::UNK) {
          // send 501 (Not Implemented)
          return this->sendBadRequest(sock, 501, "Unknown method " + methodStr);
        }
        pendingRequest.setMethod(method);
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::Uri:
    {
      if (packet.peek() == ' ') {
        packet.ignore();
        std::string uri = pendingRequest.storage;
        pendingRequest.storage.clear();
        if (uri.empty() || uri[0] != '/' || HTTP::hasDisallowedUriToken(uri)) {
          // send 400 (Bad Request)
          return this->sendBadRequest(sock, 400, "Invalid URI " + uri);
        }
        if (uri.size() > settings->get<size_t>("http.max_uri_size")) {
          // send 414 (URI Too Long)
          return this->sendBadRequest(sock, 414, "URI too long " + uri);
        }
        pendingRequest.setPath(uri);
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::Protocol:
    {
      if (packet.peek() == '/') {
        packet.ignore();
        std::string protocol = pendingRequest.storage;
        pendingRequest.storage.clear();
        if (protocol != "HTTP") {
          // send 400 (Bad Request)
          return this->sendBadRequest(sock, 400, "Invalid protocol " + protocol);
        }
        pendingRequest.setProtocol(protocol + "/");
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::VersionMajor:
    {
      if (packet.peek() == '.') {
        packet.ignore();
        std::string version = pendingRequest.storage;
        pendingRequest.storage.clear();
        if (version != "1") {
          // send 505 (HTTP Version Not Supported)
          return this->sendBadRequest(sock, 505, "Invalid version " + version);
        }
        pendingRequest.setProtocol(pendingRequest.protocol + version + ".");
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::VersionMinor:
    {
      if (packet.peek() == '\r') {
        char version = pendingRequest.storage[0];
        pendingRequest.storage.clear();
        int minor = version - '0';
        if (minor < 0 || minor > 1) {
          // send 505 (HTTP Version Not Supported)
          return this->sendBadRequest(sock, 505, "Invalid version " + Utils::toString(version));
        }
        pendingRequest.setProtocol(pendingRequest.protocol + version);
        pendingRequest.nextWithCRLF();
        continue;
      }
      break;
    }
    case ReqStates::Header:
    {
      if (packet.peek() == '\r') {
        pendingRequest.state = ReqStates::HeaderValue;
        pendingRequest.nextWithCRLF();
        continue;
      }
      pendingRequest.next();
      continue;
    }
    case ReqStates::HeaderKey:
    {
      if (std::crlf.find(packet.peek()) != std::string::npos) {
        pendingRequest.state = ReqStates::HeaderValue;
        pendingRequest.nextWithCRLF();
        continue;
      }
      if (packet.peek() == ':') {
        packet.ignore();
        std::string key = pendingRequest.storage;
        pendingRequest.storage.clear();
        if (key.empty() || HTTP::hasFieldToken(key)) {
          // send 400 (Bad Request)
          return this->sendBadRequest(sock, 400, "Invalid header key " + key);
        }
        Logger::debug
          << "Got field key " << Logger::param(key)
          << std::endl;
        pendingRequest.buildingHeaderKey = key;
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::HeaderValue:
    {
      if (packet.peek() == '\r') {
        std::string value = pendingRequest.storage;
        pendingRequest.storage.clear();
        const std::string key = pendingRequest.buildingHeaderKey;
        pendingRequest.buildingHeaderKey.clear();
        Logger::debug
          << "Got field value " << Logger::param(value)
          << " for key " << Logger::param(key)
          << std::endl;
        pendingRequest.getHeaders().append(key, value);
        pendingRequest.state = ReqStates::Header;
        pendingRequest.nextWithCRLF();
        continue;
      }
      break;
    }
    case ReqStates::HeaderEnd:
    {
      const Headers& headers = pendingRequest.getHeaders();
      if (headers.has("Transfer-Encoding") && headers.get<std::string>("Transfer-Encoding") == "chunked")
        pendingRequest.state = ReqStates::BodyChunked;
      else if (!headers.has("Content-Length") && !headers.has("Transfer-Encoding") && pendingRequest.getMethod() > Methods::GET)
        return this->sendBadRequest(sock, 411, "Missing Content-Length");
      else if (headers.get<size_t>("Content-Length") == 0)
        pendingRequest.state = ReqStates::Done;
      else if (headers.get<size_t>("Content-Length") > settings->get<size_t>("http.max_body_size"))
        return this->sendBadRequest(sock, 413, "Body too long: " + Utils::toString(headers.get<int>("Content-Length")));
      else
        pendingRequest.state = ReqStates::Body;
      continue;
    }
    case ReqStates::Body:
    {
      const size_t contentLength = pendingRequest.getContentLength();
      Logger::debug
        << "Curr body size: " << Logger::param(pendingRequest.chunkData.size())
        << " | Content-Length: " << Logger::param(contentLength)
        << std::endl;
      if (pendingRequest.chunkData.size() < contentLength)
        pendingRequest.chunkData.push_back(packet.get());
      if (pendingRequest.chunkData.size() == contentLength) {
        pendingRequest.body.append(reinterpret_cast<char*>(pendingRequest.chunkData.data()), pendingRequest.chunkData.size());
        pendingRequest.chunkData.clear();
        pendingRequest.state = ReqStates::Done;
      }
      continue;
    }
    case ReqStates::BodyChunked:
    {
      if (!std::isxdigit(packet.peek())) {
        return this->sendBadRequest(sock, 400, "Invalid chunk size");
      }
      pendingRequest.next();
      break;
    }
    case ReqStates::BodyChunkBytes:
    {
      if (packet.peek() == '\r') {
        std::stringstream ss;
        ss << pendingRequest.storage;
        pendingRequest.storage.clear();
        ss >> std::hex >> pendingRequest.chunkSize;
        Logger::debug
          << "Got chunk size " << Logger::param(pendingRequest.chunkSize)
          << std::endl;
        if (pendingRequest.chunkSize == 0)
          pendingRequest.state = ReqStates::BodyChunkData;
        pendingRequest.nextWithCRLF();
      }
      break;
    }
    case ReqStates::BodyChunkData:
    {
      if (pendingRequest.chunkData.size() < pendingRequest.chunkSize)
        pendingRequest.chunkData.push_back(packet.get());
      if (pendingRequest.chunkData.size() == pendingRequest.chunkSize) {
        pendingRequest.body.append(reinterpret_cast<char*>(pendingRequest.chunkData.data()), pendingRequest.chunkData.size());
        pendingRequest.chunkData.clear();
        pendingRequest.state = ReqStates::BodyChunked;
        pendingRequest.nextWithCRLF();
      }
      continue;
    }
    case ReqStates::BodyChunkEnd:
    {
      pendingRequest.state = ReqStates::Done;
      break;
    }
    default:
      break;
    }
  }
  Logger::debug
    << "Received packet from " << Logger::param(sock) << ": " << std::endl
    << Logger::param(pendingRequest) << std::endl;
  if (pendingRequest.getState() == ReqStates::Done) {
    this->sendBadRequest(sock, 500, "Invalid state " + ReqStates::ToString(pendingRequest.getState()));
  }
}

void WebSocket::sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg) {
  Logger::warning
    << "Sending " << statusCode << " to " << Logger::param(sock) << ": " << logMsg
    << std::endl;
  Request req(this->pendingRequests.at(sock));
  Response resp(req, NULL);
  resp.getHeaders().set("Connection", "close");
  resp.status(statusCode).send();
}