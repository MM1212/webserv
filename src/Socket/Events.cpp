#include "Socket.hpp"

using Socket::Dispatch::StartedEvent;
using Socket::Dispatch::NewConnectionEvent;
using Socket::Dispatch::DisconnectedEvent;

using Socket::Handling::StartedHandler;
using Socket::Handling::NewConnectionHandler;
using Socket::Handling::DisconnectedHandler;
using Socket::Handling::RawDataHandler;
using Events::Event;
using Socket::Connection;

StartedEvent::StartedEvent(const Server& _sock)
  : Event("socket::started"), sock(_sock) {}

StartedEvent::~StartedEvent() {}

const Socket::Server& StartedEvent::getServer() const {
  return this->sock;
}

NewConnectionEvent::NewConnectionEvent(const Connection& connection)
  : Event("socket::connection::on"), connection(connection) {}
NewConnectionEvent::~NewConnectionEvent() {}

const Connection& NewConnectionEvent::getConnection() const {
  return this->connection;
}

DisconnectedEvent::DisconnectedEvent(const Connection& connection)
  : Event("socket::connection::off"), connection(connection) {}
DisconnectedEvent::~DisconnectedEvent() {}

const Connection& DisconnectedEvent::getConnection() const {
  return this->connection;
}

StartedHandler::StartedHandler(const Events::EventListener<StartedEvent>::Handler handler)
  : Events::EventListener<StartedEvent>("socket::started", handler) {}
StartedHandler::~StartedHandler() {}

NewConnectionHandler::NewConnectionHandler(const Events::EventListener<NewConnectionEvent>::Handler handler)
  : Events::EventListener<NewConnectionEvent>("socket::connection::on", handler) {}
NewConnectionHandler::~NewConnectionHandler() {}

DisconnectedHandler::DisconnectedHandler(const Events::EventListener<DisconnectedEvent>::Handler handler)
  : Events::EventListener<DisconnectedEvent>("socket::connection::off", handler) {}
DisconnectedHandler::~DisconnectedHandler() {}

RawDataHandler::RawDataHandler(const Events::EventListener<Dispatch::DataEvent<std::string> >::Handler handler)
  : Events::EventListener<Dispatch::DataEvent<std::string> >("socket::data", handler) {}
RawDataHandler::~RawDataHandler() {}