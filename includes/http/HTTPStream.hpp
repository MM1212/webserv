#pragma once

#include <ByteStream.hpp>
#include <cstring>

namespace HTTP {
  class HTTPStream {
  public:
    HTTPStream(ByteStream& stream) : stream(stream) {}
    ~HTTPStream() {}

    const ByteStream& getStream() const {
      return this->stream;
    }
    ByteStream& getStream() {
      return this->stream;
    }
    uint64_t size() const {
      return this->stream.size();
    }
    uint8_t* data() {
      return this->stream.data();
    }
    bool getline(std::string& line) {
      line.clear();
      uint8_t* eol = reinterpret_cast<uint8_t*>(std::memchr(this->data(), '\r', this->size()));
      if (!eol)
        return false;
      if (eol + 1 >= this->data() + this->size())
        return false;
      if (*(eol + 1) != '\n')
        return false;
      line = std::string(reinterpret_cast<char*>(this->data()), eol - this->data());
      this->stream.ignore(eol - this->data() + 2);
      return true;
    }
  private:
    ByteStream& stream;
  };
}