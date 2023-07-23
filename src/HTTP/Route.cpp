#include "HTTP.hpp"

using HTTP::Route;
using HTTP::Methods;

Route::Route(
  Methods::Method method,
  const std::string& path,
  Handler handler
) :
  method(method),
  path(path),
  handler(handler) {}

Route::Route(const Route& other) :
  method(other.method),
  path(other.path),
  handler(other.handler) {}

Route::~Route() {}

Route& Route::operator=(const Route& other) {
  this->path = other.path;
  this->method = other.method;
  this->handler = other.handler;
  return *this;
}

Route::operator std::string() {
  return this->path;
}

Route::operator Methods::Method() {
  return this->method;
}

Methods::Method Route::getMethod() const {
  return this->method;
}

const std::string& Route::getPath() const {
  return this->path;
}

Route::Handler Route::getHandler() const {
  return this->handler;
}