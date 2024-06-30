#pragma once

#include <string>
#include <Yaml.hpp>
#include <iostream>

namespace HTTP {
  namespace Routing {
    struct Types {
      enum Type {
        None = -1,
        Default,
        Static,
        Redirect,
        CGI,
        Script,
      };
      static std::string ToString(Type type);
      static Type FromString(const std::string& type);
    };
    struct Middleware {
      enum Type {
        None = -1,
        Found,
        Next,
        Break
      };
      int val;
      Middleware(Type type) : val(type) {}
      Middleware(int type) : val(type) {}
      Middleware() : val(Found) {}

      inline operator int() const { return this->val; }
      inline operator Type() const {
        if (this->val > Break) return Break;
        return static_cast<Type>(this->val);
      }

      static std::string ToString(Type type);
      static Type FromString(const std::string& type);
    };

  }
}