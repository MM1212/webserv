#pragma once

#include "FileManager.hpp"
#include "Connection.hpp"
#include "Types.hpp"

#include <utils/misc.hpp>

namespace Socket {
  struct Server {
    int sock;
    std::string address;
    int port;

    inline operator int() const {
      return this->sock;
    }

    inline operator std::string() const {
      return this->address + ":" + Utils::toString(this->port);
    }
  };
  class Parallel {
  private:
    FileManager<Parallel> fileManager;
    std::map<int, Server> servers;
    std::map<int, Connection> clients;
    std::map<std::string, int> addressesToSock;
    int timeout;
  public:
    Parallel(int timeout);
    ~Parallel();

    bool hasServer(const std::string& address, const int port) const;
    bool hasServer(const Server& server) const;
    bool hasServer(const int sock) const;
    bool hasClient(const Connection& client) const;
    bool hasClient(const std::string& address, const int port) const;
    bool hasClient(const int sock) const;

    Server& getServer(const std::string& address, const int port);
    Server& getServer(const int sock);

    Connection& getClient(const std::string& address, const int port);
    Connection& getClient(const int sock);
    const Server& bind(
      const Domain::Handle domain,
      const Type::Handle type,
      const Protocol::Handle protocol,
      const std::string& address,
      const int port,
      const int backlog
    );

    bool kill(int sock);

    void disconnect(int client);
    void disconnect(const Connection& client);

    void run();

  private:

    void onTick(const std::vector<File>& changed);

    void onNewConnection(const Server& sock);
    void onClientDisconnect(const Connection& sock);
    void _onClientRead(Connection& sock);
    void _onClientWrite(Connection& sock);

    virtual void onClientRead(Connection& sock) = 0;
    virtual void onClientWrite(Connection& sock, int bytesWrote) = 0;
  };

}