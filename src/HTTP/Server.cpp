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
  routes(), socket(this),
   log("Webserv")
{}

Server::Server(const Server& other)
  :
  routes(other.routes),
  socket(other.socket),
  log(other.log) {}

Server::~Server() {}

Server& Server::operator=(const Server& other) {
  this->socket = other.socket;
  this->routes = other.routes;
  this->log = other.log;
  return *this;
}

bool Server::listen(
  const std::string& address,
  const int port
) {
  return this->socket.listen(address, port, 128, 500);
}

void Server::onStart() {
  std::cout << "Listening on " << this->socket.getAddress() << ":" << this->socket.getPort() << std::endl;
}

void Server::onNewConnection(Socket::Connection& connection) {
  std::cout << "New connection from " << connection.getAddress() << ":" << connection.getPort() << std::endl;
}

void Server::onDisconnected(Socket::Connection& connection) {
  std::cout << "Disconnected from " << connection.getAddress() << ":" << connection.getPort() << std::endl;
}

template <typename T>
void print_container(const std::vector<T>& c)
{
  std::cout << "The container c contains:" << std::endl;
  for (size_t i = 0; i < c.size(); ++i)
    std::cout << c[i] << " ";
  std::cout << '\n';

  std::cout << "End of container." << std::endl;

}

void Server::onData(Socket::Connection& connection, const std::string& buffer) {
  std::cout << "onData" << std::endl;
  std::cout << buffer << std::endl;
  std::vector<std::string> head_body = Utils::split(buffer, "\r\n\r\n");
  if (head_body.size() < 1) {
    this->log.error("Invalid request");
    connection.disconnect();
    return;
  }
  std::vector<std::string> head = Utils::split(head_body[0], "\r\n");
  if (head.size() < 1) {
    this->log.error("Invalid head size");
    connection.disconnect();
    return;
  }
  std::string first_line = head[0];
  head.erase(head.begin());
  std::vector<std::string> first_line_parts = Utils::split(first_line, " ");
  if (first_line_parts.size() < 3) {
    this->log.error("Invalid first line size");
    connection.disconnect();
    return;
  }
  Headers headers;
  for (std::vector<std::string>::iterator it = head.begin(); it != head.end(); ++it) {
    size_t pos = it->find_first_of(": ");
    if (pos == std::string::npos) {
      this->log.error("Invalid header format in %s", it->c_str());
      connection.disconnect();
      return;
    }
    std::string key = it->substr(0, pos);
    std::string value = it->substr(pos + 2);
    headers.append(key, value);
  }
  std::cout << headers << std::endl;
  Request req(
    const_cast<Server*>(this),
    const_cast<Socket::Connection*>(&connection),
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
