#include "HTTP.hpp"

using HTTP::Server;
using HTTP::Route;
using HTTP::Methods;
using HTTP::Router;

Router::Router() {}
Router::~Router() {}
Router::Router(const Router& other) {
  *this = other;
}

Router& Router::operator=(const Router& other) {
  this->routes = other.routes;
  return *this;
}

const Route& Router::search(const std::string& path, const Methods::Method method) const {
  try {
    return *this->routes.at(path).at(method);
  }
  catch (const std::exception& e) {
    throw Router::NotFoundException();
  }
}

bool Router::has(const std::string& path, const Methods::Method method) const {
  try {
    this->routes.at(path).at(method);
    return true;
  }
  catch (const std::exception& e) {
    return false;
  }
}

bool Router::has(const std::string& path) const {
  return this->routes.count(path) > 0;
}

bool Router::has(const Route& route) const {
  return this->has(route.getPath(), route.getMethod());
}

bool Router::add(const Route& route) {
  if (this->has(route))
    return false;

  const std::pair<Methods::Method, Route*> pair(route.getMethod(), const_cast<Route*>(&route));
  this->routes[route.getPath()].insert(pair);
  return true;
}

bool Router::get(Route& route) {
  if (route.getMethod() != Methods::GET)
    route.setMethod(Methods::GET);
  return this->add(route);
}

bool Router::post(Route& route) {
  if (route.getMethod() != Methods::POST)
    route.setMethod(Methods::POST);
  return this->add(route);
}

bool Router::put(Route& route) {
  if (route.getMethod() != Methods::PUT)
    route.setMethod(Methods::PUT);
  return this->add(route);
}

bool Router::del(Route& route) {
  if (route.getMethod() != Methods::DELETE)
    route.setMethod(Methods::DELETE);
  return this->add(route);
}

bool Router::head(Route& route) {
  if (route.getMethod() != Methods::HEAD)
    route.setMethod(Methods::HEAD);
  return this->add(route);
}

void Router::run(Request& req, Response& res) const {
  if (!this->has(req.getPath()))
    return res.status(404).send();
  else if (!this->has(req.getPath(), req.getMethod()))
    return res.status(405).send();
  try {
    const Route& route = this->search(req.getPath(), req.getMethod());
    route.run(req, res);
  }
  catch (const std::exception& e) {
    return res.status(500).send();
  }
}

bool Router::hookFile(FileRoute& route) {
  if (this->has(route))
    return false;
  this->add(route);
  if (route.getMethod() == Methods::GET)
    this->head(route);
  return true;
}