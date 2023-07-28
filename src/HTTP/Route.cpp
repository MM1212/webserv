#include "HTTP.hpp"

using HTTP::Route;
using HTTP::Methods;

Route::Route(
  const std::string& path
) : path(path), type(Routes::Normal) {}

Route::Route(const Route& other) : path(other.path), type(other.type) {}

Route::~Route() {}

Route& Route::operator=(const Route& other) {
  this->path = other.path;
  this->type = other.type;
  return *this;
}

Route::operator std::string() {
  return this->path;
}

const std::string& Route::getPath() const {
  return this->path;
}

void Route::setPath(const std::string& path) {
  this->path = path;
}