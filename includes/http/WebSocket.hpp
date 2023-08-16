#pragma once

#include <socket/Parallel.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>

#include "Headers.hpp"
#include "Methods.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "PendingRequest.hpp"
#include "PendingResponse.hpp"

namespace HTTP {
  class WebSocket : public Socket::Parallel {
  private:
    std::map<int, PendingRequest> pendingRequests;
    std::map<pid_t, PendingResponse> pendingCGIResponses;
    std::map<int, pid_t> pendingCGIProcesses;
  public:
    WebSocket();
    ~WebSocket();
    virtual void onRequest(const Request& req, Response& res) = 0;

    void trackCGIResponse(pid_t pid, int std[2], Response& res);
  private:
    virtual void onClientConnect(const Socket::Connection& sock);
    virtual void onClientDisconnect(const Socket::Connection& sock);
    virtual void onClientRead(Socket::Connection& sock);
    virtual void onClientWrite(Socket::Connection&, int) {}
    virtual void onProcessRead(Socket::Process& process);
    virtual void onProcessWrite(Socket::Process&, int) {}
    virtual void onProcessExit(const Socket::Process& process, bool force = false);

    void handleClientPacket(Socket::Connection& sock);

    void sendBadRequest(Socket::Connection& sock, int statusCode, const std::string& logMsg);
  };
};