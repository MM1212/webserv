#include "Socket.hpp"

const char* Socket::Errors::FailedToCreateSocket::what() const throw() {
  return "Failed to create socket";
}
