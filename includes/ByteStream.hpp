/**
 * ByteStream.hpp
 * ByteStream class using Buffer with uint8_t.
*/
#pragma once

#include <Buffer.hpp>

class ByteStream : public Buffer<uint8_t> {
public:
  ByteStream() : Buffer<uint8_t>() {}
  ByteStream(uint64_t size) : Buffer<uint8_t>(size) {}
  ByteStream(const ByteStream& other) : Buffer<uint8_t>(other) {}
  ByteStream& operator=(const ByteStream& other) {
    this->Buffer<uint8_t>::operator=(other);
    return *this;
  }
  ~ByteStream() {}

  bool getline(std::string& line) {
    line.clear();
    int64_t eol = -1;
    for (uint64_t i = 0; i < this->size(); ++i) {
      if (this->at(i) == '\n') {
        eol = i;
        break;
      }
    }
    bool eof = eol == -1;
    if (eof)
      eol = this->size();
    line = std::string(reinterpret_cast<char*>(this->data()), eol);
    this->ignore(eol + 1);
    return !eof;
  }
};