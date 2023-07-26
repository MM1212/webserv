#pragma once

#include <refs/Call.hpp>
#include "Action.hpp"

// Generic event listener that will store a CallRef and call it when triggered


namespace Events {

  template <typename T>
  class Listener {
  public:
    typedef Ref::Call<T, void, Action> Handler;
  private:
    Handler cb;
    std::string type;
  public:
    Listener() : cb(nullptr), type("") {}
    Listener(const std::string& type, const Handler& cb)
      : cb(cb), type(type) {}
    Listener(const Listener& other)
      : cb(other.cb), type(other.type) {}
    Listener& operator=(const Listener& other) {
      if (this == &other) return *this;
      this->cb = other.cb;
      this->type = other.type;
      return *this;
    };
    virtual ~Listener() {}
    virtual void run(const Action& action) {
      if (action.getType() != this->type) return;
      this->cb(action);
    }
    void operator()(const Action& action) {
      this->run(action);
    }
    inline const std::string& getType() const {
      return this->type;
    }
    inline const Handler& getCallback() const {
      return this->cb;
    }
  };
}