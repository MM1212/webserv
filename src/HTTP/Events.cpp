#include "HTTP.hpp"

using HTTP::Server;

Server::OnSocketDataHandler::OnSocketDataHandler(
  Server* server)
  :
  Events::EventListener<Socket::Dispatch::DataEvent<std::string> >("socket::data", NULL),
  server(server) {}

Server::OnSocketDataHandler::~OnSocketDataHandler() {}

typedef void (HTTP::Server::* Handler)(const Socket::Dispatch::DataEvent<std::string>&);

void Server::OnSocketDataHandler::onEvent(const Events::Event& ev) const {
  const Socket::Dispatch::DataEvent<std::string>& event = dynamic_cast<const Socket::Dispatch::DataEvent<std::string>&>(ev);
  this->server->onData(event);
}