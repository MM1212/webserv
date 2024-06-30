#pragma once

#include <string>

// SOURCE: https://www.rfc-editor.org/rfc/rfc9110.html#name-methods
namespace HTTP {
  namespace Methods {
    enum Method {
      UNK = -1,
      HEAD,
      GET,
      DELETE,
      POST,
      PUT,
    };
    Method FromString(const std::string& str);
    std::string ToString(Method method);
  };
}