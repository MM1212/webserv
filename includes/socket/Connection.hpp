#pragma once

#include <string>
#include <sstream>
#include <utils/misc.hpp>

#include "FileManager.hpp"

namespace Socket {
  class Connection {
  private:
    File& handle;
    int serverSock;
    ByteStream readBuffer;
    ByteStream writeBuffer;

    std::string address;
    int port;

    int timeout;
    uint64_t heartbeat;

    bool closeOnEmptyWriteBuffer;
  public:
    Connection(
      File& handle,
      int serverSock,
      int timeout
    );
    Connection(const Connection& other);
    ~Connection();

    const std::string& getAddress() const;
    const File& getHandle() const;
    int getServerSock() const;
    int getPort() const;
    std::string getIpAddress() const;
    ByteStream& getReadBuffer();
    ByteStream& getWriteBuffer();
    int getTimeout() const;
    uint64_t getHeartbeat() const;
    bool isAlive() const;
    bool isReadable() const;
    bool isWritable() const;
    bool hasTimedOut() const;
    void ping();

    void disconnect();

    inline operator int() const {
      return this->handle;
    }
    inline operator std::string() const {
      return this->address + ":" + Utils::toString(this->port);
    }

    inline void markToClose() {
      this->closeOnEmptyWriteBuffer = true;
    }
    inline bool shouldCloseOnEmptyWriteBuffer() const {
      return this->closeOnEmptyWriteBuffer;
    }
  private:
    void init();
  };
}