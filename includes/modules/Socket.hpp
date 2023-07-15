#pragma once

#include <shared.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>


#define SOCK_SEND(...) send(...)
#define SOCK_RECV(...) recv(...)
#define SOCK_LISTEN(...) listen(...)
#define SOCK_ACCEPT(...) accept(...)
#define SOCKET_CLOSE(fd) close(fd)

namespace Socket
{
  struct Domain {
    enum Handle {
      INET = AF_INET,
      INET6 = AF_INET6,
      UNIX = AF_UNIX,
      UNSPEC = AF_UNSPEC
    };
  };

  struct Type {
    enum Handle {
      TCP = SOCK_STREAM,
      UDP = SOCK_DGRAM,
      SEQPACKET = SOCK_SEQPACKET,
      RAW = SOCK_RAW,
      RDM = SOCK_RDM
    };
  };

  struct Protocol {
    enum Handle {
      IP = IPPROTO_IP,
      TCP = IPPROTO_TCP,
      UDP = IPPROTO_UDP,
      SCTP = IPPROTO_SCTP,
      DCCP = IPPROTO_DCCP,
      ESP = IPPROTO_ESP,
      AH = IPPROTO_AH,
      BEETPH = IPPROTO_BEETPH,
      COMP = IPPROTO_COMP,
      PIM = IPPROTO_PIM,
      SCTP = IPPROTO_SCTP,
      UDPLITE = IPPROTO_UDPLITE,
      MPLS = IPPROTO_MPLS,
      RAW = IPPROTO_RAW
    };
  };

  namespace Handling {
    struct Types {
      enum {
        OnNewConnection,
        OnDisconnect,
        OnData,
        OnError
      };
    };
  }

  namespace Errors {
    class FailedToCreateSocket : public std::exception {
    public:
      const char* what() const throw();
    };
    class FailedToBindSocket : public std::exception {
    public:
      const char* what() const throw();
    };
  }

  class Server;

  class Connection {
  private:
    int clientId;
    Server* server;
    std::string address;
    int port;
    uint32_t timeout;
    uint32_t heartbeat;
  public:
    Connection(int clientId, Server* server);
    inline const int getId() const;
    inline const Server* getServer() const;
    inline const std::string& getAddress() const;
    inline const int getPort() const;
    inline const bool isAlive() const;
    template <typename T>
    const bool send(const T& data, const int flags, const int timeout) const;
    const bool disconnect() const;
  };

  class Server {
  private:
    Domain::Handle domain = Domain::INET;
    Type::Handle type = Type::TCP;
    Protocol::Handle protocol = Protocol::IP;
    int handleFd = -1;
    int port = 0;
    std::string address;
    std::set<Connection> clients;
    std::map<Handling::Types, std::vector<void (*)(const Connection&)> > handlers;
    int connectionTimeout = 0;
    Connection accept();
    bool accept(Connection& connection);
  public:
    Server(
      const Domain::Handle domain,
      const Type::Handle type = Type::TCP,
      const Protocol::Handle protocol = Protocol::IP
    );
    ~Server();
    inline const Domain::Handle getDomain() const;
    inline const Type::Handle getType() const;
    inline const Protocol::Handle getProtocol() const;
    bool listen(
      const std::string& address,
      const int port,
      const int backlog = 10,
      const int timeout = 0);
    bool isListening() const;
    bool disconnect(const int clientId);
    bool disconnect(const Connection& connection);
    bool disconnectAll();
    bool close();
  };
}
