#include "Socket.hpp"

using Socket::Dispatch::StartedEvent;
using Socket::Dispatch::NewConnectionEvent;
using Socket::Dispatch::DisconnectedEvent;

using Socket::Handling::StartedHandler;
using Socket::Handling::NewConnectionHandler;
using Socket::Handling::DisconnectedHandler;
using Socket::Handling::RawDataHandler;
using Events::Action;
using Socket::Connection;

StartedEvent::StartedEvent(const Server& _sock)
  : Action("socket::started"), sock(_sock) {}

StartedEvent::~StartedEvent() {}

const Socket::Server& StartedEvent::getServer() const {
  return this->sock;
}

NewConnectionEvent::NewConnectionEvent(const Connection& connection)
  : Action("socket::connection::on"), connection(connection) {}
NewConnectionEvent::~NewConnectionEvent() {}

const Connection& NewConnectionEvent::getConnection() const {
  return this->connection;
}

DisconnectedEvent::DisconnectedEvent(const Connection& connection)
  : Action("socket::connection::off"), connection(connection) {}
DisconnectedEvent::~DisconnectedEvent() {}

const Connection& DisconnectedEvent::getConnection() const {
  return this->connection;
}