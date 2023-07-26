#pragma once

#include <string>

// abstracted Event action that has a type

namespace Events {
  class Action {
    Action();
  protected:
    std::string type;
    Action(const std::string& type);
    Action(const Action& other);
    Action& operator=(const Action& other);
  public:
    const std::string& getType() const;
    virtual ~Action();
  };
}