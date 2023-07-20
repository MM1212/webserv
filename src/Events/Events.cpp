#include "Events.hpp"

using Events::Event;
using Events::EventListener;
using Events::EventDispatcher;

Event::Event(const std::string& type)
  : type(type) {}
Event::~Event() {}

Event::Event(const Event& other) {
  *this = other;
}

Event& Event::operator=(const Event& other) {
  if (this == &other) return *this;
  this->type = other.type;
  return *this;
}

const std::string& Event::getType() const {
  return this->type;
}

EventDispatcher::EventDispatcher() : listeners() {}
EventDispatcher::~EventDispatcher() {}

void EventDispatcher::addEventListener(const EventListener<Event>& listener) {
  this->listeners[listener.getType()].push_back(listener);
}

void EventDispatcher::removeEventListener(const EventListener<Event>& listener) {
  std::vector<EventListener<Event> >& listeners = this->listeners[listener.getType()];
  for (
    std::vector<EventListener<Event> >::iterator it = listeners.begin();
    it != listeners.end();
    ++it
    ) {
    if (it->getHandler() == listener.getHandler()) {
      listeners.erase(it);
      break;
    }
  }
}

void EventDispatcher::dispatchEvent(const Event& event) const {
  std::map<std::string, std::vector<EventListener<Event> > >::const_iterator it = this->listeners.find(event.getType());
  if (it != this->listeners.end()) {
    for (
      std::vector<EventListener<Event> >::const_iterator it2 = it->second.begin();
      it2 != it->second.end();
      ++it2
      ) {
      it2->onEvent(event);
    }
  }
}

inline void EventDispatcher::emit(const Event& event) const {
  this->dispatchEvent(event);
}