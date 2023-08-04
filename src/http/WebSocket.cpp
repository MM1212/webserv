#include "http/WebSocket.hpp"
#include <Settings.hpp>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <http/utils.hpp>

using namespace HTTP;

typedef HTTP::PendingRequest::States ReqStates;

static Settings* settings = Instance::Get<Settings>();

WebSocket::WebSocket()
  : Socket::Parallel(settings->get<int>("keep_alive_timeout")) {}

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

  while (packet.peek() != EOF) {
    if (pendingRequest.checkCRLF) {
      std::string clrf = "";
      clrf += packet.get();
      clrf += packet.get();
      if (clrf != std::clrf) {
        // send 400 (Bad Request)
        return this->sendBadRequest(sock, 400, "Invalid CRLF at " + ReqStates::ToString(pendingRequest.getState()));
      }
      pendingRequest.next();
      continue;
    }
    switch (pendingRequest.state) {
    case ReqStates::Method:
    {
      std::string methodStr = "";
      std::getline(packet, methodStr, ' ');
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
      break;
    }
    case ReqStates::Uri:
    {
      std::string uri = "";
      std::getline(packet, uri, ' ');
      if (uri.empty() || uri[0] != '/' || HTTP::hasDisallowedUriToken(uri)) {
        // send 400 (Bad Request)
        return this->sendBadRequest(sock, 400, "Invalid URI " + uri);
      }
      if (uri.size() > settings->get<size_t>("max_uri_size")) {
        // send 414 (URI Too Long)
        return this->sendBadRequest(sock, 414, "URI too long " + uri);
      }
      pendingRequest.setPath(uri);
      pendingRequest.next();
      break;
    }
    case ReqStates::Protocol:
    {
      std::string protocol = "";
      std::getline(packet, protocol, '/');
      if (protocol != "HTTP") {
        // send 400 (Bad Request)
        return this->sendBadRequest(sock, 400, "Invalid protocol " + protocol);
      }
      pendingRequest.setProtocol(protocol + "/");
      pendingRequest.next();
      break;
    }
    case ReqStates::VersionMajor:
    {
      std::string version = "";
      std::getline(packet, version, '.');
      if (version != "1") {
        // send 505 (HTTP Version Not Supported)
        return this->sendBadRequest(sock, 505, "Invalid version " + version);
      }
      pendingRequest.setProtocol(pendingRequest.protocol + version + ".");
      pendingRequest.next();
      break;
    }
    case ReqStates::VersionMinor:
    {
      char version;
      packet >> version;
      int minor = version - '0';
      if (minor < 0 || minor > 1) {
        // send 505 (HTTP Version Not Supported)
        return this->sendBadRequest(sock, 505, "Invalid version " + Utils::toString(version));
      }
      pendingRequest.setProtocol(pendingRequest.protocol + version);
      pendingRequest.nextWithCRLF();
      break;
    }
    case ReqStates::Header:
    {
      if (packet.peek() == '\r') {
        pendingRequest.state = ReqStates::HeaderEnd;
        pendingRequest.nextWithCRLF();
        break;
      }
      pendingRequest.next();
      break;
    }
    case ReqStates::HeaderKey:
    {
      if (packet.peek() == '\r') {
        pendingRequest.state = ReqStates::HeaderValue;
        pendingRequest.nextWithCRLF();
        break;
      }
      std::string key = "";
      std::getline(packet, key, ':');
      if (key.empty() || HTTP::hasFieldToken(key)) {
        // send 400 (Bad Request)
        return this->sendBadRequest(sock, 400, "Invalid header key " + key);
      }
      Logger::debug
        << "Got field key " << Logger::param(key)
        << std::endl;
      pendingRequest.buildingHeaderKey = key;
      pendingRequest.next();
      break;
    }
    case ReqStates::HeaderValue:
    {
      std::string value = "";
      while (packet.peek() != '\r' && packet.peek() != EOF)
        value += packet.get();
      if (packet.peek() == EOF)
        return this->sendBadRequest(sock, 400, "Invalid header value " + value);
      const std::string key = pendingRequest.buildingHeaderKey;
      pendingRequest.buildingHeaderKey = "";
      Logger::debug
        << "Got field value " << Logger::param(value)
        << " for key " << Logger::param(key)
        << std::endl;
      pendingRequest.getHeaders().set(key, value);
      pendingRequest.state = ReqStates::Header;
      pendingRequest.nextWithCRLF();
      break;
    }
    case ReqStates::HeaderEnd:
    {
      const Headers& headers = pendingRequest.getHeaders();
      if (!headers.has("Content-Length") || headers.get<int>("Content-Length") == 0) {
        pendingRequest.state = ReqStates::Done;
        break;
      }
      if (headers.has("Transfer-Encoding") && headers.get<std::string>("Transfer-Encoding") == "chunked") {
        pendingRequest.state = ReqStates::BodyChunked;
      }
      break;
    }
    default:
      break;
    }
  }
  Logger::debug
    << "Received packet from " << Logger::param(sock) << ": " << std::endl
    << Logger::param(pendingRequest) << std::endl;
  this->sendBadRequest(sock, 500, "Invalid state " + ReqStates::ToString(pendingRequest.getState()));
}

void WebSocket::sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg) {
  Logger::warning
    << "Sending " << statusCode << " to " << Logger::param(sock) << ": " << logMsg
    << std::endl;
  std::stringstream writeBuffer;
  sock.markToClose();
  writeBuffer << "HTTP/1.1 " << statusCode << " " << settings->statusCode(statusCode) << "\r\n"
    << "Content-Length: 0\r\n"
    << "Connection: close\r\n"
    << "\r\n";
  sock.getWriteBuffer().append(writeBuffer.str());
}