/**
 * Buffer.hpp
 * Generic Buffer class using std::vector.
*/
#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <cstring>

template <typename T>
class Buffer {
private:
  std::vector<T> buffer;
public:
  Buffer() {}
  Buffer(uint64_t size) : buffer(size) {}
  Buffer(const Buffer<T>& other) : buffer(other.buffer) {}
  Buffer<T>& operator=(const Buffer<T>& other) {
    this->buffer = other.buffer;
    return *this;
  }
  ~Buffer() {}
  inline uint64_t size() const { return this->buffer.size(); }
  inline bool empty() const { return this->buffer.empty(); }
  inline T* data() { return this->buffer.data(); }
  inline const T* data() const { return this->buffer.data(); }
  inline void resize(uint64_t size) { this->buffer.resize(size); }
  inline void reserve(uint64_t size) { this->buffer.reserve(size); }
  inline void clear() { this->buffer.clear(); }
  inline void put(const T& value) { this->buffer.push_back(value); }
  template <typename K>
  void put(const K& value, uint64_t amount) {
    this->buffer.reserve(this->buffer.size() + amount);
    for (uint64_t i = 0; i < amount; ++i)
      this->buffer.push_back(value[i]);
  }
  inline void put(const std::vector<T>& value) { this->put(value, value.size()); }
  inline void put(const Buffer<T>& value) { this->put(value, value.size()); }
  inline void put(const std::string& value) { this->put(value, value.size()); }
  inline T& operator[](uint64_t index) { return this->buffer[index]; }
  inline const T& operator[](uint64_t index) const { return this->buffer[index]; }
  inline T& at(uint64_t index) { return this->buffer.at(index); }
  inline const T& at(uint64_t index) const { return this->buffer.at(index); }
  inline T& peek() { return this->buffer.front(); }
  inline const T& peek() const { return this->buffer.front(); }

  template <typename U>
  U peek() const {
    if constexpr (std::is_same_v<U, std::string>)
      return this->peekString();
    else if constexpr (std::is_same_v<U, int>)
      return this->_peekInt();
    std::stringstream ss;
    for (uint64_t i = 0; i < sizeof(U); ++i)
      ss << this->buffer[i];
    U value;
    ss >> value;
    return value;
  }
private:
  int _peekInt() const {
    if (this->empty()) return -1;
    return static_cast<int>(this->buffer[0]);
  }

  std::string peekString() const {
    std::stringstream ss;
    for (uint64_t i = 0; i < this->buffer.size(); ++i)
      ss << this->buffer[i];
    return ss.str();
  }
public:
  template <typename U>
  U get() {
    if constexpr (std::is_same_v<U, T>)
      return this->_getType();
    else if constexpr (std::is_same_v<U, int>)
      return this->_getInt();
    const U value = this->peek<U>();
    this->buffer.erase(this->buffer.begin());
    return value;
  }
private:
  T _getType() {
    const T& value = this->peek();
    this->buffer.erase(this->buffer.begin());
    return value;
  }
  int _getInt() {
    if (this->empty()) return -1;
    const int value = this->peek();
    this->buffer.erase(this->buffer.begin());
    return value;
  }
public:
  inline void ignore(uint64_t bytes = 1) {
    if (bytes == 0) return;
    this->buffer.erase(this->buffer.begin(), this->buffer.begin() + std::min(bytes, this->size()));
  }
  void take(Buffer<T>& value, uint64_t bytes = 1) {
    if (bytes == 0) return;
    value.clear();
    value.resize(bytes);
    std::memmove(value.data(), value.data() + bytes, value.size() - bytes);
    this->ignore(bytes);
  }

  template <typename U>
  U take() {
    if constexpr (std::is_same_v<U, std::string>)
      return this->_takeString();
    else if constexpr (std::is_same_v<U, int>)
      return this->_takeInt();
    std::stringstream ss;
    for (uint64_t i = 0; i < sizeof(U); ++i)
      ss << this->get<uint8_t>();
    U value;
    ss >> value;
    return value;
  }

private:
  int _takeInt() {
    return this->get<int>();
  }

  std::string _takeString() {
    std::string value;
    value.resize(this->size() + 1);
    for (uint64_t i = 0; i < this->size(); ++i)
      value[i] = this->get<char>();
    value[this->size()] = '\0';
    return value;
  }
public:
  std::string toString() const {
    return this->peek<std::string>();
  }

  inline Buffer<T>& operator<<(const T& value) { this->put(value); return *this; }
  inline operator std::vector<T>& () { return this->buffer; }
  inline operator const std::vector<T>& () const { return this->buffer; }
  inline operator std::vector<T>* () { return &this->buffer; }
  inline operator const std::vector<T>* () const { return &this->buffer; }
  inline operator void* () { return this->buffer.data(); }
  inline operator const void* () const { return this->buffer.data(); }
  inline operator bool() const { return this->size() > 0; }
  inline bool operator!() const { return this->size() == 0; }
};