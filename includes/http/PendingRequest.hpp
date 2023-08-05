#pragma once

#include <string>
#include <socket/Parallel.hpp>
#include <cstdlib>

#include "Methods.hpp"
#include "Headers.hpp"
#include "Request.hpp"

namespace HTTP {
  class WebSocket;

  class PendingRequest {
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
    const Headers& getHeaders() const;
    Methods::Method getMethod() const;
    const std::string& getPath() const;
    const std::string& getProtocol() const;
    const Socket::Connection& getClient() const;
    template <typename T>
    const T getBody() const {
      return static_cast<T>(this->body);
    }
    template <>
    const std::string getBody<std::string>() const {
      return this->body;
    }

    template <typename T>
    T getParam(const std::string& key) const {
      return static_cast<T>(this->params.at(key));
    }
    template <>
    std::string getParam<std::string>(const std::string& key) const {
      return this->params.at(key);
    }
    template <>
    int getParam<int>(const std::string& key) const {
      return std::atoi(this->params.at(key).c_str());
    }

    template <>
    bool getParam<bool>(const std::string& key) const {
      return this->params.at(key) == "true";
    }

    std::map<std::string, std::string>& getParams();

    void setMethod(const Methods::Method method);
    void setPath(const std::string& path);
    void setProtocol(const std::string& protocol);
    void setBody(const std::string& body);
    void addToBody(const std::string& body);
    void setHeaders(const Headers& headers);

    operator Request() const;

    friend std::ostream& operator<<(std::ostream& os, const PendingRequest& request);

    // quick helpers for request builder
    inline size_t getContentLength() const {
      return this->getHeaders().get<size_t>("Content-Length");
    }
  private:
    States::State crlfNextState;
    States::State state;
    Headers headers;
    Methods::Method method;
    std::string path;
    std::string body;
    std::string protocol;
    Socket::Parallel* server;
    Socket::Connection* client;
    std::map<std::string, std::string> params;
    std::string storage;
    std::string buildingHeaderKey;
    size_t chunkSize;
    std::vector<uint8_t> chunkData;

    PendingRequest();
    void parseParams();

    friend class WebSocket;

  };

}