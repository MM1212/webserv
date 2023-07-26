#pragma once

#include "Action.hpp"
#include "Listener.hpp"

#include <map>
#include <vector>
#include <string>


namespace Events {
  class Dispatcher {
  private:
    std::map<std::string, std::vector<Listener<void>*> > listeners;
  public:
    Dispatcher();
    Dispatcher(const Dispatcher& other);
    ~Dispatcher();
    Dispatcher& operator=(const Dispatcher& other);
    void addEventListener(Listener<void>* listener);
    template <typename T>
    void on(T* listener) {
      this->addEventListener(reinterpret_cast<Listener<void>*>(listener));
    }
    void dispatch(const Action& action);
  };
}