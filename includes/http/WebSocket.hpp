#pragma once

#include <socket/Parallel.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>

#include "Headers.hpp"
#include "Methods.hpp"
#include "Request.hpp"
#include "PendingRequest.hpp"

namespace HTTP {
  class WebSocket : public Socket::Parallel {
  private:
    std::map<int, PendingRequest> pendingRequests;
  public:
    WebSocket();
    ~WebSocket();

  private:
    virtual void onClientConnect(const Socket::Connection& sock);
    virtual void onClientDisconnect(const Socket::Connection& sock);
    virtual void onClientRead(Socket::Connection& sock);
    virtual void onClientWrite(Socket::Connection&, int) {}

    void handleClientPacket(Socket::Connection& sock);

    void sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg);
  };
};