#pragma once

#include <utils/Instance.hpp>
#include "WebSocket.hpp"

#include <vector>

namespace Socket {
  struct ListenAddress : public Socket::Host {
    int maxConnections;

    ListenAddress(const std::string& address, const int port, const int maxConnections)
      : Socket::Host(port, address), maxConnections(maxConnections) {}
  };
}

namespace HTTP {
  class ServerConfiguration;
  class ServerManager : public WebSocket {
  public:
    ~ServerManager();

    inline const char*const* getEnv() const { return this->env; }
    inline void setEnv(char** env) { this->env = env; }
    bool loadConfig(const std::string& path);
    void bindServers();
  private:
    friend class ::Instance;
    ServerManager();
    virtual void onRequest(const Request& req, Response& res);
    ServerConfiguration* selectServer(const Request& req) const;
    ServerConfiguration* getDefaultServer() const;

    void addServer(const YAML::Node& node);

    static void onSIGINT(int signum);
  private:
    std::vector<ServerConfiguration*> servers;
    ServerConfiguration* defaultServer;
    const YAML::Node root;
    char** env;
  };
};