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
  if (this->pendingCGIProcesses.count(sock.getHandle()) > 0) {
    const pid_t pId = this->pendingCGIProcesses.at(sock.getHandle());
    this->kill(this->getProcess(pId), Socket::Process::ExitCodes::ClientTimeout);
  }
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
  ByteStream packet(sock.getReadBuffer());
  sock.getReadBuffer().clear();
  pendingRequest.handlePacket(packet);
  bool skip = false;
  while (skip || pendingRequest.peek() != EOF) {
    if (skip)
      skip = false;
    /* if (std::isprint(pendingRequest.peek())) {
      Logger::debug
        << "peek: " << Logger::param<char>(pendingRequest.peek())
        << " | at: " << Logger::param(ReqStates::ToString(pendingRequest.getState()))
        << std::endl;
    }
    else {
      Logger::debug
        << "peek: " << std::hex << Logger::param(pendingRequest.peek()) << std::dec
        << " | at: " << Logger::param(ReqStates::ToString(pendingRequest.getState()))
        << std::endl;
    } */
    switch (pendingRequest.state) {
    case ReqStates::CLRFCheck:
    {
      if (
        pendingRequest.peek() == '\n' &&
        pendingRequest.extract() &&
        pendingRequest.storage == std::crlf
        ) {
        pendingRequest.reset();
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
      if (pendingRequest.peek() == ' ') {
        pendingRequest.skip();
        const std::string methodStr = pendingRequest.takeFromStorage<std::string>();
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
      if (pendingRequest.peek() == ' ') {
        pendingRequest.skip();
        std::string uri = pendingRequest.takeFromStorage<std::string>();
        if (uri.empty() || uri[0] != '/' || HTTP::hasDisallowedUriToken(uri)) {
          // send 400 (Bad Request)
          return this->sendBadRequest(sock, 400, "Invalid URI " + uri + " (disallowed token)");
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
      if (pendingRequest.peek() == '/') {
        pendingRequest.skip();
        std::string protocol = pendingRequest.takeFromStorage<std::string>();
        if (protocol != "HTTP") {
          // send 400 (Bad Request)
          return this->sendBadRequest(sock, 400, "Invalid protocol " + protocol);
        }
        pendingRequest.setProtocol(protocol);
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::VersionMajor:
    {
      if (pendingRequest.peek() == '.') {
        pendingRequest.skip();
        int version = pendingRequest.takeFromStorage<int>();
        if (version != 1) {
          // send 505 (HTTP Version Not Supported)
          return this->sendBadRequest(sock, 505, "Invalid version " + Utils::toString(version));
        }
        pendingRequest.setVersionMajor(version);
        pendingRequest.next();
        continue;
      }
      break;
    }
    case ReqStates::VersionMinor:
    {
      if (pendingRequest.peek() == '\r') {
        int version = pendingRequest.takeFromStorage<int>();
        if (version < 0 || version > 1) {
          // send 505 (HTTP Version Not Supported)
          return this->sendBadRequest(sock, 505, "Invalid version " + Utils::toString(version));
        }
        pendingRequest.setVersionMinor(version);
        pendingRequest.nextWithCRLF();
        continue;
      }
      break;
    }
    case ReqStates::Header:
    {
      if (pendingRequest.peek() == '\r') {
        pendingRequest.nextWithCRLF(ReqStates::HeaderEnd);
        continue;
      }
      pendingRequest.next();
      continue;
    }
    case ReqStates::HeaderKey:
    {
      if (std::crlf.find(pendingRequest.peek()) != std::string::npos) {
        pendingRequest.nextWithCRLF(ReqStates::HeaderEnd);
        continue;
      }
      if (pendingRequest.peek() == ':') {
        pendingRequest.skip();
        std::string key = pendingRequest.takeFromStorage<std::string>();
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
      if (pendingRequest.peek() == '\r') {
        std::string value = pendingRequest.takeFromStorage<std::string>();
        const std::string key = pendingRequest.buildingHeaderKey;
        pendingRequest.buildingHeaderKey.clear();
        Logger::debug
          << "Got field value " << Logger::param(value)
          << " for key " << Logger::param(key)
          << std::endl;
        pendingRequest.getHeaders().append(key, value);
        pendingRequest.nextWithCRLF(ReqStates::Header);
        continue;
      }
      break;
    }
    case ReqStates::HeaderEnd:
    {
      Headers& headers = pendingRequest.getHeaders();
      if (headers.has("Transfer-Encoding") && headers.get<std::string>("Transfer-Encoding") == "chunked")
        pendingRequest.state = ReqStates::BodyChunked;
      else if (!headers.has("Content-Length") && !headers.has("Transfer-Encoding") && pendingRequest.getMethod() > Methods::DELETE)
        return this->sendBadRequest(sock, 411, "Missing Content-Length");
      else if (!headers.has("Content-Length") || headers.get<size_t>("Content-Length") == 0)
        pendingRequest.state = ReqStates::Done;
      // else if (headers.get<size_t>("Content-Length") > settings->get<size_t>("http.max_body_size"))
      //   return this->sendBadRequest(sock, 413, "Body too long: " + Utils::toString(headers.get<int>("Content-Length")));
      else
        pendingRequest.state = ReqStates::Body;
      if (pendingRequest.isExpecting()) {
        const Request req(pendingRequest);
        Response res(req, NULL);
        Logger::info
          << "New request expecting from " << Logger::param(sock) << ": " << std::endl
          << Logger::param(req);
        this->setClientToWrite(sock);
        this->onRequest(req, res);
        if (res.getStatus() != 100) {
          this->pendingRequests.erase(sock);
          this->pendingRequests.insert(std::make_pair(sock, PendingRequest(this, &sock)));
          return;
        }
        else
          headers.remove("Expect");
      }
      continue;
    }
    case ReqStates::Body:
    {
      const size_t contentLength = pendingRequest.getContentLength();
      // Logger::debug
      //   << "Content-Length: " << Logger::param(contentLength) << std::endl
      //   << "chunk size: " << Logger::param(pendingRequest.chunkData.size()) << std::endl
      //   << "storage size: " << Logger::param(pendingRequest.storage.size()) << std::endl;
      if (pendingRequest.chunkData.size() == contentLength) {
        pendingRequest.setBody(pendingRequest.chunkData);
        pendingRequest.reset(true);
        pendingRequest.state = ReqStates::Done;
      }
      break;
    }
    case ReqStates::BodyChunked:
    {
      if (!std::isxdigit(pendingRequest.peek())) {
        return this->sendBadRequest(sock, 400, "Invalid chunk size");
      }
      pendingRequest.next();
      continue;
    }
    case ReqStates::BodyChunkBytes:
    {
      if (pendingRequest.peek() == '\r') {
        std::stringstream ss;
        ss << pendingRequest.takeFromStorage<std::string>();
        ss >> std::hex >> pendingRequest.chunkSize;
        Logger::debug
          << "Got chunk size " << Logger::param(pendingRequest.chunkSize)
          << std::endl;
        if (pendingRequest.chunkSize == 0)
          pendingRequest.nextWithCRLF(ReqStates::BodyChunkEnd);
        else
          pendingRequest.nextWithCRLF();
        continue;
      }
      break;
    }
    case ReqStates::BodyChunkData:
    {
      // Logger::debug
      //   << "Got chunk data of size " << Logger::param(pendingRequest.chunkData.size())
      //   << " and remaining size " << Logger::param(pendingRequest.chunkSize)
      //   << std::endl;
      if (pendingRequest.chunkData.size() == pendingRequest.chunkSize) {
        pendingRequest.addToBody(pendingRequest.chunkData);
        pendingRequest.chunkData.clear();
        pendingRequest.nextWithCRLF(ReqStates::BodyChunked);
      }
      break;
    }
    case ReqStates::BodyChunkEnd:
    {
      pendingRequest.state = ReqStates::Done;
      break;
    }
    default:
      break;
    }
    pendingRequest.extract();
  }
  // Logger::debug
  //   << "Got packet from " << Logger::param(sock) << " at state "
  //   << Logger::param(ReqStates::ToString(pendingRequest.getState())) << ": " << std::endl
  //   << Logger::param(pendingRequest) << std::endl;
  if (pendingRequest.lastCheck()) {
    // handle multiform-data
    // pendingRequest.handleMultiformData();
    const Request req(pendingRequest);
    Response res(req, NULL);
    Logger::info
      << "New request from " << Logger::param(sock) << ": " << std::endl
      << Logger::param(req);
    this->setClientToWrite(sock);
    this->onRequest(req, res);
    this->pendingRequests.erase(sock);
    this->pendingRequests.insert(std::make_pair(sock, PendingRequest(this, &sock)));
  }
}

void WebSocket::sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg) {
  Logger::warning
    << "Sending " << Logger::param(statusCode) << " to " << Logger::param(sock) << ": " << logMsg
    << std::endl;
  Request req(this->pendingRequests.at(sock));
  Response resp(req, NULL);
  resp.getHeaders().set("Connection", "close");
  this->setClientToWrite(sock);
  resp.status(statusCode).send();
  this->pendingRequests.erase(sock);
  this->pendingRequests.insert(std::make_pair(sock, PendingRequest(this, &sock)));
}

void WebSocket::trackCGIResponse(pid_t pid, int std[2], Response& res) {
  if (this->pendingCGIResponses.count(pid) > 0)
    return;
  if (!this->trackProcess(pid, res.getRequest().getClient(), std))
    return;
  Socket::Process& process = this->getProcess(pid);
  process.getWriteBuffer() = res.getRequest().getRawBody();
  const_cast<ByteStream&>(res.getRequest().getRawBody()).clear();
  this->pendingCGIResponses.insert(
    std::make_pair(
      pid,
      (PendingResponse) {
    res.getRequest(), res
  }));
  PendingResponse& pending = this->pendingCGIResponses.at(pid);
  pending.response = Response(pending.request, res.getRoute());
  const Socket::File& client = res.getRequest().getClient().getHandle();
  this->pendingCGIProcesses.insert(std::make_pair(client, pid));
  Logger::debug
    << "Tracking cgi response to client " << Logger::param(client)
    << " with stds " << Logger::param(std[0]) << " and " << Logger::param(std[1])
    << " to process id " << Logger::param(pid) << std::endl;
}

void WebSocket::onProcessRead(Socket::Process& process) {
  if (this->pendingCGIResponses.count(process.getId()) == 0)
    return;
  PendingResponse& pending = this->pendingCGIResponses.at(process.getId());
  Response& res = pending.response;
  const_cast<ByteStream&>(res.getRawBody()).put(process.getReadBuffer());
  process.getReadBuffer().clear();
}

void WebSocket::onProcessExit(const Socket::Process& process, Socket::Process::ExitCodes::Code code) {
  PendingResponse& pending = this->pendingCGIResponses.at(process.getId());
  Response& res = pending.response;
  Logger::debug
    << "Creating CGI response for client: " << Logger::param(process.getClient())
    << " because of " << Socket::Process::ExitCodes::ToString(code) << std::endl;
  this->pendingCGIProcesses.erase(process.getClient());
  this->setClientToWrite(const_cast<Socket::Connection&>(process.getClient()));
  switch (code) {
  case Socket::Process::ExitCodes::Normal: {
    const HTTP::Routing::Module* mod = res.getRoute()->getModule(Routing::Types::CGI);
    if (mod) {
      const HTTP::Routing::CGI* cgi = reinterpret_cast<const HTTP::Routing::CGI*>(mod);
      try {
        if (cgi)
          cgi->handleResponse(res);
        return;
      }
      catch (const std::exception& e) {
        Logger::error
          << "Error while handling CGI response: " << e.what()
          << std::endl;
        res.status(500);
      }
    }
    else
      res.status(500);
    break;
  }
  case Socket::Process::ExitCodes::Force:
    res.status(500);
    break;
  case Socket::Process::ExitCodes::Timeout:
    res.status(504);
    break;
  case Socket::Process::ExitCodes::ClientTimeout:
    res.status(408);
    break;
  default:
    res.status(500);
    break;
  }
  res.send("");
  this->pendingCGIResponses.erase(process.getId());
}