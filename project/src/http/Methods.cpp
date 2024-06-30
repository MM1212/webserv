#include "http/Methods.hpp"
#include <utils/misc.hpp>

using namespace HTTP;

Methods::Method Methods::FromString(const std::string& str) {
  if (str == "GET") return GET;
  if (str == "POST") return POST;
  if (str == "PUT") return PUT;
  if (str == "DELETE") return DELETE;
  if (str == "HEAD") return HEAD;
  return UNK;
}

std::string Methods::ToString(Method method) {
  switch (method) {
    case UNK: return "UNK";
    case GET: return "GET";
    case POST: return "POST";
    case PUT: return "PUT";
    case DELETE: return "DELETE";
    case HEAD: return "HEAD";
  }
  return "UNK" + Utils::toString(method);
}