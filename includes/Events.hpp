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

  class EventListener {
  public:
    typedef void (*Handler)(const Event&);
  protected:
    std::string type;
    Handler handler;
  private:
    EventListener();
  protected:
    EventListener(const std::string& _type, Handler _handler);
  public:
    EventListener(const EventListener& other);
    EventListener& operator=(const EventListener& other);
    virtual ~EventListener();
    inline const std::string& getType() const;
    inline Handler getHandler() const;
    virtual void onEvent(const Event& event) const;
    void operator()(const Event& event) const;
  };

  class EventDispatcher {
  public:
    EventDispatcher();
    virtual ~EventDispatcher();
    void addEventListener(const EventListener& listener);
    inline void on(const EventListener& listener) {
      this->addEventListener(listener);
    }
    template <typename T, typename K>
    inline void on(const K& handler) {
      const T listener = T(handler);
      this->on(reinterpret_cast<const EventListener&>(listener));
    }
    void removeEventListener(const EventListener& listener);
    inline void off(const EventListener& listener) {
      this->removeEventListener(listener);
    }
    template <typename T, typename K>
    inline void off(const K& handler) {
      const T listener = T(handler);
      this->off(reinterpret_cast<const EventListener&>(listener));
    }
    void dispatchEvent(const Event& event) const;
    inline void emit(const Event& event) const;
  private:
    std::map<std::string, std::vector<EventListener > > listeners;
  };
}