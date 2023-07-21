#include "Socket.hpp"

const char* Socket::Errors::FailedToCreateSocket::what() const throw() {
  return "Failed to create socket";
}


const char* Socket::Errors::FailedToBindSocket::what() const throw() {
  return "Failed to bind socket";
}