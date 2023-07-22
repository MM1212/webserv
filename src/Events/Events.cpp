#include "Events.hpp"
#include "HTTP.hpp"

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

EventListener::EventListener(const std::string& type, Handler handler)
  : type(type), handler(handler) {}
EventListener::~EventListener() {}
EventListener::EventListener(const EventListener& other) {
  *this = other;
}
EventListener& EventListener::operator=(const EventListener& other) {
  if (this == &other) return *this;
  this->type = other.type;
  this->handler = other.handler;
  return *this;
}
const std::string& EventListener::getType() const {
  return this->type;
}

EventListener::Handler EventListener::getHandler() const {
  return this->handler;
}

void EventListener::onEvent(const Event& event) const {
  this->handler(event);
}

void EventListener::operator()(const Event& event) const {
  this->onEvent(event);
}

EventDispatcher::EventDispatcher() : listeners() {}
EventDispatcher::~EventDispatcher() {}

void EventDispatcher::addEventListener(const EventListener& listener) {
  this->listeners[listener.getType()].push_back(listener);
}

void EventDispatcher::removeEventListener(const EventListener& listener) {
  std::vector<EventListener>& listeners = this->listeners[listener.getType()];
  for (
    std::vector<EventListener>::iterator it = listeners.begin();
    it != listeners.end();
    ++it
    ) {
    if (it->getHandler() == listener.getHandler()) {
      listeners.erase(it);
      break;
    }
  }
}

inline void EventDispatcher::emit(const Event& event) const {
  this->dispatchEvent(event);
}

void EventDispatcher::dispatchEvent(const Event& event) const {
  std::map<std::string, std::vector<EventListener > >::const_iterator it = this->listeners.find(event.getType());
  if (it != this->listeners.end()) {
    for (
      std::vector<EventListener >::const_iterator it2 = it->second.begin();
      it2 != it->second.end();
      ++it2
      ) {
      const HTTP::Server::OnSocketDataHandler& test = reinterpret_cast<const HTTP::Server::OnSocketDataHandler&>(*it2);
      (void)test;
      it2->onEvent(event);
    }
  }
}