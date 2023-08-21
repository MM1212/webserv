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

namespace HTTP {
  class WebSocket;

  class PendingRequest : public Request {
  public:
    struct States {
      enum State {
        Unk = -2,
        CLRFCheck = -1,
        Method,
        Uri,
        Protocol,
        VersionMajor,
        VersionMinor,
        Header,
        HeaderKey,
        HeaderValue,
        HeaderEnd,
        Body,
        BodyChunked,
        BodyChunkBytes,
        BodyChunkData,
        BodyChunkEnd,
        BodyEnd,
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
    void nextWithCRLF(int nextState = -1);
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
    void addToBody(const ByteStream& body);
    void setHeaders(const Headers& headers);

    friend std::ostream& operator<<(std::ostream& os, const PendingRequest& request);

    // quick helpers for request builder
    inline size_t getContentLength() const {
      return this->getHeaders().get<size_t>("Content-Length");
    }

    inline void handlePacket(ByteStream& packet) {
      this->cPacket = &packet;
    }
    inline bool isParsingBody() const {
      return this->state == States::Body || this->state == States::BodyChunkData;
    }
    bool extract();
    inline bool skip(uint32_t bytes = 1) {
      if (this->peek() == EOF) return false;
      this->ignore(bytes);
      return true;
    }
    inline void reset(bool clearChunkData = false) {
      this->storage.clear();
      if (clearChunkData) this->chunkData.clear();
    }
    inline int peek() const {
      return this->cPacket->peek<int>();
    }

    inline int get() {
      return this->cPacket->get<int>();
    }

    inline void ignore(uint64_t bytes = 1) {
      this->cPacket->ignore(bytes);
    }

    template <typename T>
    inline T takeFromStorage() {
      std::stringstream ss(this->storage);
      T tmp;
      ss >> tmp;
      this->storage.clear();
      return tmp;
    }
    template <>
    inline std::string takeFromStorage<std::string>() {
      std::string tmp = this->storage;
      this->storage.clear();
      return tmp;
    }

    bool lastCheck();
  private:
    States::State crlfNextState;
    States::State state;
    std::string storage;
    std::string buildingHeaderKey;
    size_t chunkSize;
    ByteStream chunkData;
    ByteStream* cPacket;

    PendingRequest();
    friend class WebSocket;
  };

}