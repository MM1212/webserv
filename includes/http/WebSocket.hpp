/**
 * WebSocket.hpp
 * The HTTP::WebSocket class is used to manage multiple pending HTTP::Request & HTTP::Response.
 * It uses the Socket::Parallel class with inheritance to manage all opened connections (sockets) & cgi processes.
 * When a socket packet is received, the class will parse it as an HTTP Request.
 * While building the request, if something is wrong, the class will send a HTTP::Response and close the connection.
 * If the HTTP Request is valid, the class will call the onRequest method.
*/
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