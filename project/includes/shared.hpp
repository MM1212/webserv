/**
 * shared.hpp
 * Some shared imports & std definitions.
*/
#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <exception>
#include <error.h>
#include <cstdlib>
#include <cstdio>
#include <utils/std.hpp>
#include <ByteStream.hpp>
#include <string_view>

namespace std {
  static const string_view crlf("\r\n");
  static const char newl = '\n';
}