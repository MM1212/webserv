/**
 * Parallel.hpp
 * The Socket::Parallel class is used to manage multiple sockets at once.
 * It uses the Socket::FileManager class to manage file descriptors.
 * It also manages processes pipes.
 * When a file descriptor is read, the onTick event is called.
 * If the file descriptor is a client socket, the onClient methods are called.
 * If the file descriptor is a server socket, the onNewConnection method is called.
 * If the file descriptor is a process pipe, the onProcess methods are called.
*/

#pragma once

#include "FileManager.hpp"
#include "Connection.hpp"
#include "Process.hpp"
#include "Types.hpp"

#include <utils/misc.hpp>

namespace Socket {
  struct Server : public Host {
    int sock;
    uint64_t maxConnections;
    uint64_t connections;

    inline operator int() const {
      return this->sock;
    }
    Server(const int sock, const std::string& address, const int port, const uint64_t maxConnections)
      : Host(port, address), sock(sock), maxConnections(maxConnections), connections(0) {}
    inline void newConnection() { this->connections++; }
    inline void onDisconnect() { this->connections = std::max(0, (int)this->connections - 1); }

  };
  class Parallel {
  private:
    FileManager<Parallel> fileManager;
    std::map<int, Server> servers;
    std::map<int, Connection> clients;
    std::map<std::string, int> addressesToSock;
    int timeout;

    std::map<pid_t, Process> processes;
    std::map<int, pid_t> pipesToProcesses;
  public:
    Parallel(int timeout);
    virtual ~Parallel();

    bool hasServer(const std::string& address, const int port) const;
    bool hasServer(const Server& server) const;
    bool hasServer(const int sock) const;
    bool hasClient(const Connection& client) const;
    bool hasClient(const std::string& address, const int port) const;
    bool hasClient(const int sock) const;

    bool hasProcess(const pid_t pid) const;
    bool hasProcess(const Process& process) const;
    bool hasProcessBoundTo(const int pipeFd) const;

    Server& getServer(const std::string& address, const int port);
    Server& getServer(int sock);
    const Server& getServer(int sock) const;

    Connection& getClient(const std::string& address, const int port);
    Connection& getClient(const int sock);

    Process& getProcess(const pid_t pid);
    const Process& getProcess(const pid_t pid) const;
    Process& getProcessBoundTo(const int pipeFd);
    const Process& getProcessBoundTo(const int pipeFd) const;
    bool trackProcess(const pid_t pid, const Connection& client, int std[2]);

    const Server& bind(
      const Domain::Handle domain,
      const Type::Handle type,
      const Protocol::Handle protocol,
      const Host& host,
      const int backlog
    );

    bool kill(int sock);

    void disconnect(int client);
    void disconnect(const Connection& client);

    void kill(const Process& process, Process::ExitCodes::Code code = Process::ExitCodes::Force);

    void run();
    inline void stop() {
      this->fileManager.stop();
    }

    void setClientToRead(Connection& client);
    void setClientToWrite(Connection& client);

  private:

    void onTick(const std::vector<File>& changed);

    void _onNewConnection(Server& sock);
    void _onClientDisconnect(const Connection& sock);
    void _onClientRead(Connection& sock);
    void _onClientWrite(Connection& sock);
    bool _onClientEmptyBuffer(Connection& sock);
    void _onProcessRead(Process& process);
    void _onProcessWrite(Process& process);
    bool _onProcessEmptyBuffer(Process& process);
    void _onProcessExit(Process& process, Process::ExitCodes::Code code = Process::ExitCodes::Force);

  protected:
    virtual void onClientConnect(const Connection& sock) = 0;
    virtual void onClientDisconnect(const Connection& sock) = 0;
    virtual void onClientRead(Connection& sock) = 0;
    virtual void onClientWrite(Connection& sock, int bytesWritten) = 0;
    virtual void onProcessRead(Process& process) = 0;
    virtual void onProcessWrite(Process& process, int bytesWritten) = 0;
    virtual void onProcessExit(const Process& process, Process::ExitCodes::Code code = Process::ExitCodes::Force) = 0;
  };

}