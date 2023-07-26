#pragma once

#include <shared.hpp>

// Templated call reference that supports simple functions or member methods

namespace Ref {

  template <typename T, typename R, typename A>
  class Call {
  public:
    typedef R(T::* Handler)(const A& param);
  private:
    T* instance;
    Handler handler;
  private:
    Call() : instance(nullptr), handler(nullptr) {}
  public:
    Call(T* instance, Handler handler)
      : instance(instance), handler(handler) {}
    Call(Handler handler)
      : instance(nullptr), handler(handler) {}
    Call(const Call& other)
      : instance(other.instance), handler(other.handler) {}
    Call& operator=(const Call& other) {
      if (this == &other) return *this;
      this->instance = other.instance;
      this->handler = other.handler;
      return *this;
    }
    R operator()(const A& param) {
      if (!this->instance)
        return (*this->handler)(param);
      else if (!this->handler)
        throw std::runtime_error("Call: no handler");
      return (this->instance->*this->handler)(param);
    }
    inline Handler getHandler() const {
      return this->handler;
    }
    inline T* getInstance() const {
      return this->instance;
    }
  };

}