#pragma once

#include "Request.hpp"
#include "Response.hpp"

namespace HTTP {
  struct PendingResponse {
    const Request request;
    Response response;
  };
}