#pragma once

#include <string>

namespace HTTP {
  namespace Methods {
    enum Method {
      UNK = -1,
      GET,
      POST,
      PUT,
      DELETE,
      HEAD
    };
    Method FromString(const std::string& str);
    std::string ToString(Method method);
  };
}