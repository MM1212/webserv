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

#define nullptr NULL

namespace std {
  template<bool>
  struct static_assertion;

  template<>
  struct static_assertion<true> {};

  static std::string crlf("\r\n");
}