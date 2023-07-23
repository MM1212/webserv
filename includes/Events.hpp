#pragma once

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <iostream>

namespace Events {
  class Event {
  private:
    Event();
    Event(const Event& other);
    virtual Event& operator=(const Event& other);
  protected:
    Event(const std::string& type);
  public:
    virtual ~Event();
    const std::string& getType() const;
  private:
    std::string type;
  };

  template <typename T>
  class EventListener {
  public:
    typedef void (*Handler)(const T&);
  private:
    std::string type;
    Handler handler;
  private:
    EventListener() : type(""), handler(NULL) {}
  protected:
    EventListener(const std::string& _type, Handler _handler)
      : type(_type), handler(_handler) {}
  public:
    EventListener(const EventListener<T>& other) : type(other.type), handler(other.handler) {}
    EventListener<T>& operator=(const EventListener<T>& other) {
      this->type = other.type;
      this->handler = other.handler;
      return *this;
    }
    virtual ~EventListener() {}
    inline const std::string& getType() const {
      return this->type;
    }
    inline const Handler getHandler() const {
      return this->handler;
    }
    void virtual onEvent(const Event& event) const {
      const T& ev = dynamic_cast<const T&>(event);
      this->handler(ev);
    }
    inline void operator()(const Event& event) const {
      this->onEvent(event);
    }
  };

  class EventDispatcher {
  public:
    EventDispatcher();
    virtual ~EventDispatcher();
    void addEventListener(const EventListener<Event>& listener);
    template <typename T>
    inline void on(const EventListener<T>& listener) {
      const EventListener<Event>& l = reinterpret_cast<const EventListener<Event>&>(listener);
      this->addEventListener(l);
    }
    template <typename T, typename K>
    inline void on(const K& handler) {
      const T listener = T(handler);
      this->on(listener);
    }
    void removeEventListener(const EventListener<Event>& listener);
    template <typename T>
    inline void off(const EventListener<Event>& listener) {
      const EventListener<Event>& l = reinterpret_cast<const EventListener<Event>&>(listener);
      this->removeEventListener(l);
    }
    template <typename T, typename K>
    inline void off(const K& handler) {
      const T listener = T(handler);
      this->off(listener);
    }
    void dispatchEvent(const Event& event) const;
    inline void emit(const Event& event) const;
  private:
    std::map<std::string, std::vector<EventListener<Event> > > listeners;
  };
}