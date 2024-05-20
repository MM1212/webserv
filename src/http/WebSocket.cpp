#include "http/WebSocket.hpp"
#include <Settings.hpp>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <http/utils.hpp>
#include <http/ErrorCodes.hpp>

using namespace HTTP;

typedef HTTP::PendingRequest::States ReqStates;

static Settings* settings = Instance::Get<Settings>();

WebSocket::WebSocket()
  : Socket::Parallel(settings->get<int>("socket.keep_alive_timeout")) {}

WebSocket::~WebSocket() {}

void WebSocket::onClientConnect(const Socket::Connection&) {}

bool WebSocket::onClientDisconnect(Socket::Connection& sock) {
  if (this->pendingRequests.count(sock) > 0 && sock.hasTimedOut()) {
    this->sendBadRequest(sock, ErrorCodes::RequestTimeout, "Client timed out");
    return false;
  }
  if (this->pendingCGIProcesses.count(sock.getHandle()) > 0) {
    const pid_t pId = this->pendingCGIProcesses.at(sock.getHandle());
    this->kill(this->getProcess(pId), Socket::Process::ExitCodes::ClientTimeout);
  }
  this->pendingRequests.erase(sock);
  return true;
}

void WebSocket::onClientRead(Socket::Connection& sock) {
  this->handleClientPacket(sock);
}

void WebSocket::handleClientPacket(Socket::Connection& sock) {
  if (!this->pendingRequests.count(sock))
    this->pendingRequests.insert(std::make_pair(sock, PendingRequest(this, &sock)));
  PendingRequest& pendingRequest = this->pendingRequests.at(sock);
  ByteStream& packet = sock.getReadBuffer();
  HTTPStream stream(packet);
  pendingRequest.handlePacket(stream);
  while (pendingRequest.getState() != ReqStates::Done) {
    if (!pendingRequest.isParsingBody()) {
      std::string line;
      if (!stream.getline(line))
        break;
      Logger::debug
        << "Parsing line: " << Logger::param(line)
        << " from " << Logger::param(sock)
        << " in state " << Logger::param(ReqStates::ToString(pendingRequest.getState()))
        << std::newl;
      switch (pendingRequest.getState())
      {
      case ReqStates::Uri: {

        std::vector<std::string> parts = Utils::split(line, " ");
        if (parts.size() != 3)
          return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid URI " + line + " (invalid parts)");
        { // Handle Method
          std::string& methodStr = parts[0];
          const Methods::Method method = Methods::FromString(methodStr);
          Logger::debug
            << "method: " << Logger::param(methodStr) << " (" << Logger::param(method) << ")"
            << std::newl;
          if (method == Methods::UNK) {
            // send 501 (Not Implemented)
            return this->sendBadRequest(sock, ErrorCodes::NotImplemented, "Unknown method " + methodStr);
          }
          pendingRequest.setMethod(method);
        }
        { // Handle URL
          std::string& path = parts[1];
          Logger::debug
            << "path: " << Logger::param(path)
            << std::newl;
          if (path.empty() || path[0] != '/' || HTTP::hasDisallowedUriToken(path)) {
            // send 400 (Bad Request)
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid URI " + line + " (disallowed token)");
          }
          if (path.size() > settings->get<size_t>("http.max_uri_size")) {
            // send 414 (URI Too Long)
            return this->sendBadRequest(sock, ErrorCodes::URITooLong, "URI too long " + path);
          }
          pendingRequest.setPath(path);
        }
        { // Handle Protocol
          std::string& protocol = parts[2];
          std::vector<std::string> protocolParts = Utils::split(protocol, "/");
          if (protocolParts.size() != 2)
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid protocol " + protocol);
          pendingRequest.setProtocol(protocol);
          PendingRequest::Protocol proto = pendingRequest.getProtocol();
          Logger::debug
            << "protocol: " << Logger::param(proto.type) << " ("
            << Logger::param(proto.versionMajor) << "." << Logger::param(proto.versionMinor)
            << ")" << std::newl;
          if (proto.type != "HTTP")
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid protocol " + proto.type);
          if (proto.versionMajor != 1 || proto.versionMinor != 1)
            return this->sendBadRequest(sock, ErrorCodes::HTTPVersionNotSupported, "Invalid protocol version " + static_cast<std::string>(proto));
        }
        pendingRequest.next();
        break;
      }
      case ReqStates::Headers: {
        if (line.empty()) {
          Headers& headers = pendingRequest.getHeaders();
          if (headers.has("Transfer-Encoding") && headers.get<std::string>("Transfer-Encoding") == "chunked")
            pendingRequest.state = ReqStates::BodyChunkSize;
          else if (!headers.has("Content-Length") && !headers.has("Transfer-Encoding") && pendingRequest.getMethod() > Methods::DELETE)
            return this->sendBadRequest(sock, ErrorCodes::LengthRequired, "Missing Content-Length");
          else if (!headers.has("Content-Length") || headers.get<size_t>("Content-Length") == 0)
            pendingRequest.state = ReqStates::Done;
          else
            pendingRequest.state = ReqStates::Body;
          if (pendingRequest.isExpecting()) {
            const Request req(pendingRequest);
            Response res(req, NULL);
            Logger::info
              << "New request expecting from " << Logger::param(sock) << ": " << std::newl
              << Logger::param(req);
            this->setClientToWrite(sock);
            this->onRequest(req, res);
            if (res.getStatus() != ErrorCodes::Continue) {
              this->pendingRequests.erase(sock);
              return;
            }
            else
              headers.remove("Expect");
          }
          break;
        }
        std::string::size_type sepPos = line.find(':');
        if (sepPos != std::string::npos) { // handle ':' case
          if (sepPos == 0 || sepPos == line.size() - 1)
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid header " + line);
          std::string key = line.substr(0, sepPos);
          std::string value = line.substr(sepPos + 1);
          if (HTTP::hasFieldToken(key))
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid header " + key);
          pendingRequest.getHeaders().append(key, value);
          Logger::debug
            << "new header variable: " << Logger::param(key) << " = " << Logger::param(value)
            << std::newl;
          break;
        }
        sepPos = line.find(';');
        if (sepPos != std::string::npos) { // handle ';' case
          std::string key = line.substr(0, sepPos);
          if (HTTP::hasFieldToken(key))
            return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid header " + key);
          pendingRequest.getHeaders().append(key, "");
          Logger::debug
            << "new header variable: " << Logger::param(key)
            << std::newl;
          break;
        }
        return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid header " + line);
      }
      case ReqStates::BodyChunkSize: {
        std::stringstream ss(line);
        if (!(ss >> std::hex >> pendingRequest.chunkSize))
          return this->sendBadRequest(sock, ErrorCodes::BadRequest, "Invalid chunk size " + line);
        if (pendingRequest.chunkSize == 0) {
          pendingRequest.state = ReqStates::Done;
          break;
        }
        pendingRequest.next();
        break;
      }
      default:
        break;
      }
    }
    else {
      switch (pendingRequest.getState()) {
        case ReqStates::Body: {
          size_t size = std::min(packet.size(), pendingRequest.getHeaders().get<size_t>("Content-Length"));
          packet.resize(size);
          pendingRequest.addToBody(packet);
          if (pendingRequest.getBody().size() == pendingRequest.getHeaders().get<size_t>("Content-Length"))
            pendingRequest.state = ReqStates::Done;
          break;
        }
        case ReqStates::BodyChunkData: {
          size_t size = std::min(packet.size(), pendingRequest.chunkSize);
          pendingRequest.chunkData.put(packet, size);
          if (pendingRequest.chunkData.size() == pendingRequest.chunkSize) {
            pendingRequest.addToBody(pendingRequest.chunkData);
            pendingRequest.chunkData.clear();
            pendingRequest.state = ReqStates::BodyChunkSize;
          }
          break;
        }
        default:
          break;
      }
    }
  }
  if (pendingRequest.lastCheck()) {
    // handle multiform-data
    // pendingRequest.handleMultiformData();
    const Request req(pendingRequest);
    Response res(req, NULL);
    Logger::info
      << "New request from " << Logger::param(sock) << ": " << std::newl
      << Logger::param(req);
    this->setClientToWrite(sock);
    this->onRequest(req, res);
    this->pendingRequests.erase(sock);
  }
}

void WebSocket::sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg) {
  Logger::warning
    << "Sending " << Logger::param(statusCode) << " to " << Logger::param(sock) << ": " << logMsg
    << std::newl;
  PendingRequest& pendingRequest = this->pendingRequests.at(sock);
  pendingRequest.setBody("");
  pendingRequest.setProtocol("HTTP/1.1");
  Request req(pendingRequest);
  Response resp(req, NULL);
  resp.getHeaders().set("Connection", "close");
  this->setClientToWrite(sock);
  resp.status(statusCode).send();
  this->pendingRequests.erase(sock);
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
    << " to process id " << Logger::param(pid) << std::newl;
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
    << " because of " << Socket::Process::ExitCodes::ToString(code) << std::newl;
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
          << std::newl;
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