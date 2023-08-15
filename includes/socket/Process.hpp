#pragma once

#include <string>
#include <sstream>
#include <utils/misc.hpp>
#include <sys/types.h>
#include <signal.h>

#include "FileManager.hpp"

namespace Socket {
  class Process {
  private:
    File& in;
    File& out;
    pid_t id;

    std::stringstream readBuffer;
    std::string writeBuffer;
  public:
    Process(File& in, File& out, pid_t id);
    Process(const Process& other);
    ~Process();

    inline pid_t getId() const { return this->id; }

    inline bool isAlive() const { return ::kill(this->id, 0) == 0; }
    inline const File& getIn() const { return this->in; }
    inline const File& getOut() const { return this->out; }

    inline bool isReadable() const {
      return this->in.isReadable();
    }
    inline bool isWritable() const {
      return this->out.isWritable();
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