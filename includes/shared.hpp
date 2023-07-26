#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <exception>

#define nullptr NULL

namespace std {
  template<bool>
  struct static_assertion;

  template<>
  struct static_assertion<true> {};
}