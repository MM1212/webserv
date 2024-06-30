/**
 * Process.hpp
 * Similar to Connection.hpp.
 * Stores a CGI process and refs to its pipes (they live in FileManager).
 * Also has some helper methods & timeout functionality.
*/
#pragma once

#include <string>
#include <sstream>
#include <utils/misc.hpp>
#include <sys/types.h>
#include <csignal>
#include <shared.hpp>

#include "FileManager.hpp"
#include "Connection.hpp"

namespace Socket {
  class Process {
  public:
    struct ExitCodes {
      enum Code {
        Normal,
        Force,
        Timeout,
        ClientTimeout
      };
      static std::string ToString(Code code);
    };
  private:
    const File* in;
    const File* out;
    const Connection& client;
    int std[2];
    pid_t id;

    ByteStream readBuffer;
    ByteStream writeBuffer;
    int timeout;
    uint64_t heartbeat;

  public:
    Process(
      File& in,
      File& out,
      const Connection& con,
      pid_t id,
      int timeout);
    Process(const Process& other);
    ~Process();

    inline pid_t getId() const { return this->id; }

    inline bool isAlive() const { return ::kill(this->id, 0) == 0; }
    inline bool hasIn() const { return this->in != nullptr; }
    inline bool hasOut() const { return this->out != nullptr; }
    inline const Connection& getClient() const { return this->client; }
    inline const File& getIn() const { return *this->in; }
    inline const File& getOut() const { return *this->out; }
    inline const int* getRawStd() const { return this->std; }
    inline bool hasTimedOut() const {
      return Utils::getCurrentTime() - this->heartbeat >= static_cast<uint32_t>(this->timeout);
    }
    inline void ping() { this->heartbeat = Utils::getCurrentTime(); }

    inline void removeIn() { this->in = nullptr; }
    inline void removeOut() { this->out = nullptr; }

    inline bool isReadable() const {
      return this->in->isReadable();
    }
    inline bool isWritable() const {
      return this->out->isWritable();
    }

    inline ByteStream& getReadBuffer() {
      return this->readBuffer;
    };
    inline ByteStream& getWriteBuffer() {
      return this->writeBuffer;
    }

    void kill();
    void write(const ByteStream& buff);

    inline operator bool() const { return this->isAlive(); }
    inline operator pid_t() const { return this->id; }
  };
}