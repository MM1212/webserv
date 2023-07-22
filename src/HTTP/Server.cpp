#include "HTTP.hpp"
#include <utils/misc.hpp>

using HTTP::Server;
using HTTP::Route;
using HTTP::Methods;
using HTTP::Request;
using HTTP::Response;
using HTTP::Headers;

Server::Server()
  :
  socket(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP),
  routes() {}

Server::Server(const Server& other)
  :
  socket(other.socket),
  routes(other.routes) {}

Server::~Server() {}

Server& Server::operator=(const Server& other) {
  this->socket = other.socket;
  this->routes = other.routes;
  return *this;
}

void Server::listen(
  const std::string& address,
  const int port
) {
  const OnSocketDataHandler listener(this);
  this->socket.dispatcher.on<OnSocketDataHandler>(listener);
  this->socket.listen(address, port, 128, 500);
}

Events::EventDispatcher& Server::getSocketDispatcher() {
  return this->socket.dispatcher;
}

void Server::onData(const Socket::Dispatch::DataEvent<std::string>& ev) {
  std::cout << "onData" << std::endl;
  std::cout << ev.getData() << std::endl;
  std::vector<std::string> head_body = Utils::split(ev.getData(), "\r\n\r\n");
  if (head_body.size() < 1) {
    ev.getConnection().disconnect();
    return;
  }
  std::vector<std::string> head = Utils::split(head_body[0], "\r\n");
  if (head.size() < 1) {
    ev.getConnection().disconnect();
    return;
  }
  std::string first_line = head[0];
  head.erase(head.begin());
  std::vector<std::string> first_line_parts = Utils::split(first_line, " ");
  if (first_line_parts.size() < 3) {
    ev.getConnection().disconnect();
    return;
  }
  Headers headers;
  for (std::vector<std::string>::iterator it = head.begin(); it != head.end(); ++it) {
    size_t pos = it->find(": ");
    if (pos == std::string::npos) {
      ev.getConnection().disconnect();
      return;
    }
    headers.append(it->substr(0, pos), it->substr(pos + 2));
  }
  Request req(
    const_cast<Server*>(this),
    const_cast<Socket::Connection*>(&ev.getConnection()),
    Methods::fromString(first_line_parts[0]),
    first_line_parts[1],
    head_body[1],
    first_line_parts[2],
    headers
  );
  Response res(req);
  std::map<Methods::Method, Route>& routes = this->routes[req.getPath()];
  if (routes.count(req.getMethod()) == 0) {
    res.setStatus(404, "Not Found").send();
    return;
  }
  Route& route = routes.at(req.getMethod());
  route.getHandler()(req, res);
}