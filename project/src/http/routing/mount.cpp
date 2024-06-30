#include "http/routing/routing.hpp"

using namespace HTTP::Routing;

Module* Module::Create(const std::string& type, const Route& route, const YAML::Node& node) {
  Types::Type mType = Types::FromString(type);
  return Create(mType, route, node);
}

Module* Module::Create(const Types::Type type, const Route& route, const YAML::Node& node) {
  switch (type) {
  case Types::Static: return new Static(route, node);
  case Types::Redirect: return new Redirect(route, node);
    case Types::CGI: return new CGI(route, node);
  default: return nullptr;
  }
}