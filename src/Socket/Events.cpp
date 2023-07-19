#include "Socket.hpp"

using Socket::Dispatch::NewConnectionEvent;
using Socket::Handling::NewConnectionHandler;
using Events::Event;
using Socket::Connection;

NewConnectionEvent::NewConnectionEvent(const Connection& connection)
  : Event("socket::newConnection"), connection(connection) {}
NewConnectionEvent::~NewConnectionEvent() {}

inline const Connection& NewConnectionEvent::getConnection() const {
  return this->connection;
}

NewConnectionHandler::NewConnectionHandler(void (*callback)(const Connection&))
  : Events::EventListener<void (*)(const Connection&)>(callback) {}

void NewConnectionHandler::onEvent(const Events::Event& e) const {
  const NewConnectionEvent& ev = dynamic_cast<const NewConnectionEvent&>(e);
  this->handler(ev.getConnection());
}
