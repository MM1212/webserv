#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <exception>
#include <error.h>
#include <stdio.h>
#include <utils/std.hpp>
#include <ByteStream.hpp>

#define nullptr NULL

namespace std {
  template<bool>
  struct static_assertion;

  template<>
  struct static_assertion<true> {};

  template <typename T, typename U>
  struct is_same {
    static const bool value = false;
  };

  template <typename T>
  struct is_same<T, T> {
    static const bool value = true;
  };

  static std::string crlf("\r\n");
}