#include "Events.hpp"

using Events::Event;
using Events::EventListener;
using Events::EventDispatcher;

Event::Event(const std::string& type)
  : type(type) {}
Event::~Event() {}

const std::string& Event::getType() const {
  return this->type;
}

EventDispatcher::EventDispatcher() : listeners() {}
EventDispatcher::~EventDispatcher() {}

void EventDispatcher::addEventListener(const std::string& type, EventListener<void*>* listener) {
  this->listeners[type].push_back(listener);
}

void EventDispatcher::removeEventListener(const std::string& type, EventListener<void*>* listener) {
  std::vector<EventListener<void*>*>& listeners = this->listeners[type];
  for (
    std::vector<EventListener<void*>*>::iterator it = listeners.begin();
    it != listeners.end();
    ++it
    ) {
    if (*it == listener) {
      listeners.erase(it);
      break;
    }
  }
}

void EventDispatcher::dispatchEvent(const Event& event) const {
  std::map<std::string, std::vector<EventListener<void*>*> >::const_iterator it = this->listeners.find(event.getType());
  if (it != this->listeners.end()) {
    for (
      std::vector<EventListener<void*>*>::const_iterator it2 = it->second.begin();
      it2 != it->second.end();
      ++it2
      ) {
      (*it2)->onEvent(event);
    }
  }
}

inline void EventDispatcher::emit(const Event& event) const {
  this->dispatchEvent(event);
}