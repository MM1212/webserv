#include "events/Dispatcher.hpp"

using Events::Dispatcher;

Dispatcher::Dispatcher() : listeners() {}
Dispatcher::Dispatcher(const Dispatcher& other) : listeners(other.listeners) {}
Dispatcher::~Dispatcher() {}
Dispatcher& Dispatcher::operator=(const Dispatcher& other) {
  if (this == &other) return *this;
  this->listeners = other.listeners;
  return *this;
}

void Dispatcher::addEventListener(Listener<void>* listener) {
  const std::string& type = listener->getType();
  this->listeners[type].push_back(listener);
}

void Dispatcher::dispatch(const Action& action) {
  const std::string& type = action.getType();
  if (this->listeners.find(type) == this->listeners.end()) return;
  std::vector<Listener<void>*>& listeners = this->listeners[type];
  for (
    std::vector<Listener<void>*>::iterator it = listeners.begin();
    it != listeners.end();
    ++it
  ) 
    (*it)->run(action);
}