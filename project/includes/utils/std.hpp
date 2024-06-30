/**
 * std.hpp
 * Add features that aren't present in cpp98.
*/
#pragma once

#include <sstream>
#include <iostream>
#include <netinet/in.h>

namespace std {

  template <typename T>
  class default_delete {
  public:
    default_delete() {}
    void operator()(T* ptr) const { delete ptr; }
  };

  template <typename T>
  class default_delete<T[]> {
  public:
    default_delete() {}
    void operator()(T* ptr) const { delete[] ptr; }
  };

  template <typename T, typename Deleter = default_delete<T> >
  class unique_ptr {
  public:
    unique_ptr(T* ptr) : ptr(ptr) {}
    unique_ptr() : ptr(new T) {}
    ~unique_ptr() { this->deleter(this->ptr); }
    T* operator->() { return this->ptr; }
    T& operator*() { return *this->ptr; }
    T& operator[](int i) { return this->ptr[i]; }

    inline operator T* () { return this->ptr; }
    inline operator const T* () const { return this->ptr; }
    inline operator void* () { return this->ptr; }
    inline operator const void* () const { return this->ptr; }

    inline T* get() { return this->ptr; }
    inline const T* get() const { return this->ptr; }

    inline void reset(T* ptr) {
      this->deleter(this->ptr);
      this->ptr = ptr;
    }

    inline void reset() {
      this->deleter(this->ptr);
      this->ptr = new T;
    }

    inline T* release() {
      T* ptr = this->ptr;
      this->ptr = NULL;
      return ptr;
    }

    friend std::ostream& operator<<(std::ostream& os, const unique_ptr<T>& ptr) {
      os << ptr.get();
      return os;
    }

    friend std::stringstream& operator<<(std::stringstream& os, const unique_ptr<T>& ptr) {
      os << ptr.get();
      return os;
    }
  private:
    T* ptr;
    Deleter deleter;
  };


  template <typename T>
  class unique_ptr<T[], default_delete<T[]> > {
  public:
    unique_ptr(T* ptr) : ptr(ptr) {}
    unique_ptr(size_t size) : ptr(new T[size]) {}
    ~unique_ptr() { this->deleter(this->ptr); }
    T* operator->() { return this->ptr; }
    T& operator*() { return *this->ptr; }
    T& operator[](int i) { return this->ptr[i]; }

    inline operator T* () { return this->ptr; }
    inline operator const T* () const { return this->ptr; }
    inline operator void* () { return this->ptr; }
    inline operator const void* () const { return this->ptr; }

    inline T* get() { return this->ptr; }
    inline const T* get() const { return this->ptr; }

    inline void reset(T* ptr) {
      this->deleter(this->ptr);
      this->ptr = ptr;
    }

    inline void reset() {
      this->deleter(this->ptr);
      this->ptr = new T;
    }

    inline T* release() {
      T* ptr = this->ptr;
      this->ptr = NULL;
      return ptr;
    }

    friend std::ostream& operator<<(std::ostream& os, const unique_ptr<T[]>& ptr) {
      os << ptr.get();
      return os;
    }

    friend std::stringstream& operator<<(std::stringstream& os, const unique_ptr<T[]>& ptr) {
      os << ptr.get();
      return os;
    }
  private:
    T* ptr;
    default_delete<T[]> deleter;
  };

  in_addr_t inet_addr(const char* str);
  std::string inet_ntoa(struct in_addr addr);

}