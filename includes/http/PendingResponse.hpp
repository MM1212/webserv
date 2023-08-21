/**
 * PendingResponse.hpp
 * Small workaround to store a Request & a Response together for CGI scripts.
*/
#pragma once

#include "Request.hpp"
#include "Response.hpp"

namespace HTTP {
  struct PendingResponse {
    const Request request;
    Response response;
  };
}