#include "http/routing/types.hpp"

using namespace HTTP::Routing;

std::string Types::ToString(Type type) {
  switch (type) {
  case Static: return "Static";
  case Redirect: return "Redirect";
  case CGI: return "CGI";
  case Default: return "Default";
  default: return "Unknown";
  }
}

Types::Type Types::FromString(const std::string& type) {
  if (type == "static") return Static;
  if (type == "redirect") return Redirect;
  if (type == "cgi") return CGI;
  if (type == "default") return Default;
  return None;
}

std::string Middleware::ToString(Type type) {
  switch (type) {
  case Found: return "Found";
  case Next: return "Next";
  case Break: return "Break";
  default: return "Unknown";
  }
}

Middleware::Type Middleware::FromString(const std::string& type) {
  if (type == "found") return Found;
  if (type == "next") return Next;
  if (type == "break") return Break;
  return None;
}
