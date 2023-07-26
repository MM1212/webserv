#include "events/Action.hpp"

using Events::Action;

const std::string& Action::getType() const {
  return this->type;
}

Action::Action() : type("") {}

Action::Action(const std::string& _type) : type(_type) {}

Action::Action(const Action& other) : type(other.type) {}

Action& Action::operator=(const Action& other) {
  if (this == &other) return *this;
  this->type = other.type;
  return *this;
}

Action::~Action() {}