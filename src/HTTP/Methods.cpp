#include "HTTP.hpp"

using HTTP::Methods;

Methods::Method Methods::fromString(const std::string& str) {
  if (str == "GET") return GET;
  if (str == "POST") return POST;
  if (str == "PUT") return PUT;
  if (str == "DELETE") return DELETE;
  if (str == "HEAD") return HEAD;
  return GET;
}