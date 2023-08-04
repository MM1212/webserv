#pragma once

#include <socket/Parallel.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>

#include "Headers.hpp"
#include "Methods.hpp"
#include "Request.hpp"
#include "PendingRequest.hpp"

namespace HTTP {
  class WebSocket : private Socket::Parallel {
    private:
      std::map<int, PendingRequest> pendingRequests;
  };
};