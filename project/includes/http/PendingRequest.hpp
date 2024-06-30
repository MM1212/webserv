/**
 * PendingRequest.hpp
 * Extends HTTP::Request to add a state machine to parse the request.
 * It adds a bunch of setters.
 * It parses an socket packet byte by byte*.
 * It also has a bunch of helpers to build a request.
 * It is used by the HTTP::WebSocket class.
 * If anything goes wrong while parsing, it will send a HTTP::Response and close the connection.
 * *: It parses byte by byte until it ends the headers.
 *    Then it parses the body in chunks (min between packet size & size left) depending on the Content-Length header or ChunkSize.
*/
#pragma once

#include <string>
#include <socket/Parallel.hpp>
#include <cstdlib>

#include "Methods.hpp"
#include "Headers.hpp"
#include "Request.hpp"
#include "HTTPStream.hpp"

namespace HTTP {
  class WebSocket;

  class PendingRequest : public Request {
  public:
    struct States {
      enum State {
        Uri,
        Headers,
        Body,
        BodyChunkSize,
        BodyChunkData,
        Done
      };
      static std::string ToString(State state);
    };
    struct Protocol {
      std::string type;
      int versionMajor;
      int versionMinor;

      inline operator std::string() const {
        return this->type + "/" + Utils::toString(this->versionMajor) + "." + Utils::toString(this->versionMinor);
      }
    };
  public:
    PendingRequest(
      Socket::Parallel* server,
      Socket::Connection* client
    );
    ~PendingRequest();
    PendingRequest(const PendingRequest& other);
    PendingRequest& operator=(const PendingRequest& other);

    States::State getState() const;
    void setState(const States::State state);
    void next();
    Headers& getHeaders();
    using Request::getHeaders;

    void setMethod(const Methods::Method method);
    void setPath(const std::string& path);
    using Request::getProtocol;
    Protocol getProtocol() const;
    void setProtocol(const std::string& protocol);

    void setProtocolType(const std::string& protocolType);
    void setVersionMajor(const int versionMajor);
    void setVersionMinor(const int versionMinor);
    void setBody(const std::string& body);
    void setBody(const ByteStream& body);
    void addToBody(const std::string& body);
    void addToBody(const ByteStream& body, size_t size = -1);
    void setHeaders(const Headers& headers);

    friend std::ostream& operator<<(std::ostream& os, const PendingRequest& request);

    // quick helpers for request builder
    inline size_t getContentLength() const {
      return this->getHeaders().get<size_t>("Content-Length");
    }

    inline void handlePacket(HTTPStream& packet) {
      this->cPacket = &packet;
    }
    inline bool isParsingBody() const {
      return this->state == States::Body || this->state == States::BodyChunkData;
    }
    inline void reset(bool clearChunkData = false) {
      if (clearChunkData) this->chunkData.clear();
    }

    bool lastCheck();
  private:
    States::State state;
    std::string buildingHeaderKey;
    size_t chunkSize;
    ByteStream chunkData;
    HTTPStream* cPacket;

    PendingRequest();
    friend class WebSocket;
  };

}