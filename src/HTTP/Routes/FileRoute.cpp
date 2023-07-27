#include "HTTP.hpp"

using namespace HTTP;

FileRoute::FileRoute(Methods::Method method, const std::string& path, const std::string& filePath)
  : Route(method, path), filePath(filePath) {}

FileRoute::FileRoute(const FileRoute& other)
  : Route(other), filePath(other.filePath) {}

FileRoute::~FileRoute() {}

FileRoute& FileRoute::operator=(const FileRoute& other) {
  Route::operator=(other);
  this->filePath = other.filePath;
  return *this;
}

void FileRoute::sendHeaders(Response& res) const {
  std::ifstream file(this->filePath.c_str());
  if (!file.is_open() || !file.good())
    return res.status(404).send();
  file.close();
  res.status(200).sendHeader();
}

void FileRoute::run(Request& req, Response& res) const {
  switch (req.getMethod()) {
  case Methods::GET:
    res.status(200).sendFile(this->filePath);
    break;
  case Methods::HEAD:
    this->sendHeaders(res);
    break;
  default:
    res.status(405).send();
    break;
  }
}