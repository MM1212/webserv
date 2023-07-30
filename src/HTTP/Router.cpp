#include "HTTP.hpp"

using HTTP::Server;
using HTTP::Route;
using HTTP::Methods;
using HTTP::Router;

Router::Router() {}
Router::~Router() {
  for (
    std::map<std::string, std::map<Methods::Method, Route*> >::iterator it = this->routes.begin();
    it != this->routes.end();
    it++
    ) {
    for (
      std::map<Methods::Method, Route*>::iterator it2 = it->second.begin();
      it2 != it->second.end();
      it2++
      ) {
      if (it2->second) {
        delete it2->second;
        it->second.at(it2->first) = NULL;
      }
    }
  }
}
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

bool Router::has(const Route& route, const Methods::Method method) const {
  return this->has(route.getPath(), method);
}

bool Router::add(const Route& route, const Methods::Method method) {
  if (this->has(route, method))
    return false;

  this->routes[route.getPath()].insert(std::make_pair(method, const_cast<Route*>(&route)));
  return true;
}

bool Router::get(const Route& route) {
  return this->add(route, Methods::GET);
}

bool Router::post(const Route& route) {
  return this->add(route, Methods::POST);
}

bool Router::put(const Route& route) {
  return this->add(route, Methods::PUT);
}

bool Router::del(const Route& route) {
  return this->add(route, Methods::DELETE);
}

bool Router::head(const Route& route) {
  return this->add(route, Methods::HEAD);
}

void Router::run(Request& req, Response& res) const {
  Methods::Method selector = req.getMethod();
  if (!this->has(req.getPath()))
    return res.status(404).send();
  else if (!this->has(req.getPath(), selector)) {
    if (selector == Methods::HEAD && this->has(req.getPath(), Methods::GET))
      selector = Methods::GET;
    else
      return res.status(405).send();
  }
  try {
    const Route& route = this->search(req.getPath(), selector);
    route.run(req, res);
  }
  catch (const std::exception& e) {
    return res.status(500).send();
  }
}