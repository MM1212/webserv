#include "HTTP.hpp"

using HTTP::Route;
using HTTP::Methods;

Route::Route(
  Methods::Method method,
  const std::string& path
) :
  method(method),
  path(path) {}

Route::Route(const Route& other) :
  method(other.method),
  path(other.path) {}

Route::~Route() {}

Route& Route::operator=(const Route& other) {
  this->path = other.path;
  this->method = other.method;
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

void Route::setMethod(Methods::Method method) {
  this->method = method;
}

void Route::setPath(const std::string& path) {
  this->path = path;
}