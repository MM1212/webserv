#pragma once

#include <string>
#include <cstring>
#include <stdint.h>

template <typename T>
class Buffer {
  void* data;
  size_t size;
public:
  Buffer() : data(NULL), size(0) {}
  Buffer(const T& data) : data((void*)&data), size(sizeof(T)) {}
  Buffer(const Buffer<T>& other) : data(other.data), size(other.size) {}
  Buffer<T>& operator=(const Buffer<T>& other) {
    this->data = other.data;
    this->size = other.size;
    return *this;
  }
  ~Buffer() {}
  inline const T& getData() const { return *reinterpret_cast<T*>(this->data); }
  inline size_t getSize() const { return this->size; }
  operator const T& () const { return this->getData(); }
};

template <>
class Buffer<uint8_t> {
  uint8_t* data;
  size_t size;
public:
  Buffer() : data(NULL), size(0) {}
  Buffer(const uint8_t* data, const size_t size) : data(new uint8_t[size]), size(size) {
    std::memmove(this->data, data, size);
  }
  Buffer(const Buffer<uint8_t>& other) : data(other.data), size(other.size) {}
  Buffer<uint8_t>& operator=(const Buffer<uint8_t>& other) {
    this->data = other.data;
    this->size = other.size;
    return *this;
  }
  ~Buffer() { delete[] this->data; }
  inline const uint8_t* getData() const { return this->data; }
  inline size_t getSize() const { return this->size; }
  operator const uint8_t* () const { return this->getData(); }
  uint8_t operator[](int i) const {
    if (i < 0 || i >= static_cast<int>(this->size))
      throw std::runtime_error("Buffer index out of bounds");
    return this->data[i];
  }
};