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

    void handleMultiformData();

    inline void handlePacket(ByteStream& packet) {
      this->cPacket = &packet;
    }
    inline bool isParsingBody() const {
      return this->state == States::Body || this->state == States::BodyChunkData;
    }
    inline bool extract() {
      if (this->peek() == EOF) return false;
      if (this->isParsingBody()) {

        uint64_t bytesToRead = this->getContentLength() - this->chunkData.size();
        if (bytesToRead > this->cPacket->size()) bytesToRead = this->cPacket->size();
        ByteStream tmp;
        this->cPacket->take(tmp, bytesToRead);
        std::cout << "tmp has " << tmp.size() << " bytes" << std::endl;
        this->chunkData.put(tmp);
        std::cout << "Extracting " << bytesToRead << " bytes from packet. ChunkData now has: " << this->chunkData.size() << std::endl;
      }
      else
        this->storage += static_cast<char>(this->get());
      return true;
    }
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