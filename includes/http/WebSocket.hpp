#pragma once

#include <socket/Parallel.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>

#include "Headers.hpp"
#include "Methods.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "PendingRequest.hpp"

namespace HTTP {
  class WebSocket : public Socket::Parallel {
  private:
    std::map<int, PendingRequest> pendingRequests;
  public:
    WebSocket();
    ~WebSocket();
    virtual void onRequest(const Request& req, Response& res) = 0;
  private:
    virtual void onClientConnect(const Socket::Connection& sock);
    virtual void onClientDisconnect(const Socket::Connection& sock);
    virtual void onClientRead(Socket::Connection& sock);
    virtual void onClientWrite(Socket::Connection&, int) {}

    void handleClientPacket(Socket::Connection& sock);

    void sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg);
  };
};