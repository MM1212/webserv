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
    Event& operator=(const Event& other);
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
  protected:
    T handler;
  private:
    EventListener() : handler(NULL) {}
    EventListener(const EventListener<T>& other) : handler(other.handler) {}
    EventListener<T>& operator=(const EventListener<T>& other) {
      this->handler = other.handler;
      return *this;
    }
  public:
    EventListener(T handler) : handler(handler) {}
    virtual ~EventListener() {}
    virtual void onEvent(const Event& event) const = 0;
    inline void operator()(const Event& event) const {
      this->onEvent(event);
    }
  };

  class EventDispatcher {
  public:
    EventDispatcher();
    virtual ~EventDispatcher();
    void addEventListener(const std::string& type, EventListener<void*>* listener);
    template <typename T>
    inline void on(const std::string& type, T* listener) {
      this->addEventListener(type, reinterpret_cast<EventListener<void*>*>(listener));
    }
    void removeEventListener(const std::string& type, EventListener<void*>* listener);
    template <typename T>
    inline void off(const std::string& type, T* listener) {
      this->removeEventListener(type, reinterpret_cast<EventListener<void*>*>(listener));
    }
    void dispatchEvent(const Event& event) const;
    inline void emit(const Event& event) const;
  private:
    std::map<std::string, std::vector<EventListener<void*>*> > listeners;
  };
}