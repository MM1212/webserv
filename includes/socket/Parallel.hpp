#pragma once

#include "FileManager.hpp"
#include "Connection.hpp"
#include "Types.hpp"

#include <utils/misc.hpp>

namespace Socket {
  struct Server : public Host {
    int sock;

    inline operator int() const {
      return this->sock;
    }
    Server(const int sock, const std::string& address, const int port)
      : Host(port, address), sock(sock) {}
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
    virtual ~Parallel();

    bool hasServer(const std::string& address, const int port) const;
    bool hasServer(const Server& server) const;
    bool hasServer(const int sock) const;
    bool hasClient(const Connection& client) const;
    bool hasClient(const std::string& address, const int port) const;
    bool hasClient(const int sock) const;

    Server& getServer(const std::string& address, const int port);
    Server& getServer(int sock);

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

    void setClientToRead(Connection& client);
    void setClientToWrite(Connection& client);

  private:

    void onTick(const std::vector<File>& changed);

    void _onNewConnection(const Server& sock);
    void _onClientDisconnect(const Connection& sock);
    void _onClientRead(Connection& sock);
    void _onClientWrite(Connection& sock);

  protected:
    virtual void onClientConnect(const Connection& sock) = 0;
    virtual void onClientDisconnect(const Connection& sock) = 0;
    virtual void onClientRead(Connection& sock) = 0;
    virtual void onClientWrite(Connection& sock, int bytesWritten) = 0;

  };

}