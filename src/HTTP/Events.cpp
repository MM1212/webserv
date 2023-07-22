#include "HTTP.hpp"

using HTTP::Server;

Server::OnSocketDataHandler::OnSocketDataHandler(
  Server* server)
  :
  Events::EventListener("socket::data", NULL),
  server(server) {}

Server::OnSocketDataHandler::~OnSocketDataHandler() {}

typedef void (HTTP::Server::* Handler)(const Socket::Dispatch::DataEvent<std::string>&);

void Server::OnSocketDataHandler::onEvent(const Events::Event& ev) const {
  std::cout << "OnSocketDataHandler" << std::endl;
  const Socket::Dispatch::DataEvent<std::string>& event = dynamic_cast<const Socket::Dispatch::DataEvent<std::string>&>(ev);
  this->server->onData(event);
}