#pragma once

#include <string>

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
  operator const T&() const { return this->getData(); }
};