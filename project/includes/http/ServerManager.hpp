/**
 * ServerManager.hpp
 * The HTTP::ServerManager class is a Singleton that manages multiple ServerConfigurations.
 * It uses the HTTP::WebSocket class with inheritance to create & manage all pending requests.
 * When a HTTP Request is received, the class will call the onRequest method.
 * It will choose the best ServerConfiguration to handle the request.
 * If no ServerConfiguration is found, it will use the default one.
 * Upon selecting the ServerConfiguration, it will call the onRequest method.
*/
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
    inline uint64_t getEnvSize() const { return this->envSize; }
    void setEnv(char** env);
    bool loadConfig(const std::string& path);
    void bindServers();
  private:
    friend class ::Instance;
    ServerManager();
    virtual void onRequest(const Request& req, Response& res);
    ServerConfiguration* selectServer(const Request& req) const;
    ServerConfiguration* getDefaultServer(const Socket::Host host) const;

    void addServer(const YAML::Node& node);

    static void onSIGINT(int signum);
  private:
    std::vector<ServerConfiguration*> servers;
    std::map<Socket::Host, ServerConfiguration*> defaultServers;
    const YAML::Node root;
    char** env;
    uint64_t envSize;
  };
};