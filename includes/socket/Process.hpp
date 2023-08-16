#pragma once

#include <string>
#include <sstream>
#include <utils/misc.hpp>
#include <sys/types.h>
#include <signal.h>

#include "FileManager.hpp"
#include "Connection.hpp"

namespace Socket {
  class Process {
  private:
    const File* in;
    const File* out;
    const Connection& client;
    int std[2];
    pid_t id;

    std::stringstream readBuffer;
    std::string writeBuffer;
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

    inline void removeIn() { this->in = nullptr; }
    inline void removeOut() { this->out = nullptr; }

    inline bool isReadable() const {
      return this->in->isReadable();
    }
    inline bool isWritable() const {
      return this->out->isWritable();
    }

    inline std::stringstream& getReadBuffer() {
      return this->readBuffer;
    };
    inline std::string& getWriteBuffer() {
      return this->writeBuffer;
    }

    void kill();
    void write(const std::string& buff);

    inline operator bool() const { return this->isAlive(); }
    inline operator pid_t() const { return this->id; }
  };
}