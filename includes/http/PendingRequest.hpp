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
    void nextWithCRLF();
    void next();
    Headers& getHeaders();
    using Request::getHeaders;

    void setMethod(const Methods::Method method);
    void setPath(const std::string& path);
    void setProtocol(const std::string& protocol);
    void setBody(const std::string& body);
    void addToBody(const std::string& body);
    void setHeaders(const Headers& headers);

    friend std::ostream& operator<<(std::ostream& os, const PendingRequest& request);

    // quick helpers for request builder
    inline size_t getContentLength() const {
      return this->getHeaders().get<size_t>("Content-Length");
    }

    void handleMultiformData();
  private:
    States::State crlfNextState;
    States::State state;
    std::string storage;
    std::string buildingHeaderKey;
    size_t chunkSize;
    std::vector<uint8_t> chunkData;

    PendingRequest();
    friend class WebSocket;
  };

}