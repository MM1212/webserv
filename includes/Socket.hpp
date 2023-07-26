#pragma once

#include <events.hpp>
#include <Buffer.hpp>

#include <shared.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

#define SYS_SEND ::send
#define SYS_RECV ::recv
#define SYS_LISTEN ::listen
#define SYS_ACCEPT ::accept
#define SYS_CLOSE ::close
#define SYS_FNCTL ::fcntl
#define SYS_POLL ::poll

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

  class Server;
  class Connection {
  public:
    struct IO {
      bool read;
      bool write;
      bool error;
      int raw;
    };

  private:
    int clientId;
    Server* server;
    std::string address;
    int port;
    uint32_t timeout;
    uint32_t heartbeat;
    Connection();
  public:
    std::string buffer;
    Connection(const int clientId, Server* server);
    Connection(const Connection& other);
    Connection& operator=(const Connection& other);
    int getId() const;
    Server* getServer() const;
    const std::string& getAddress() const;
    int getPort() const;
    // @deprecated
    bool isAlive() const;
    IO poll(const int timeout) const;
    bool send(const void* data, const uint32_t size, const int flags, const int timeout) const;
    bool disconnect() const;
    void ping();
    bool timedOut() const;

    operator int() const;
  };

  namespace Dispatch {
    class StartedEvent {
    private:
      const Server& sock;
    public:
      StartedEvent(const Server& sock);
      ~StartedEvent();
      const Server& getServer() const;
    };

    class NewConnectionEvent {
    private:
      const Connection& connection;
    public:
      NewConnectionEvent(const Connection& connection);
      ~NewConnectionEvent();
      const Connection& getConnection() const;
    };

    class DisconnectedEvent {
    private:
      const Connection& connection;
    public:
      DisconnectedEvent(const Connection& connection);
      ~DisconnectedEvent();
      const Connection& getConnection() const;
    };

    template <typename T>
    class DataEvent {
    private:
      const Connection& connection;
      T* data;
    public:
      DataEvent(const Connection& connection, T* data)
        : connection(connection), data(data) {}
      inline const Connection& getConnection() const {
        return this->connection;
      }
      inline const T& getData() const {
        return *this->data;
      }
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

  class Server {
  private:
    Domain::Handle domain;
    Type::Handle type;
    Protocol::Handle protocol;
    int handleFd;
    int port;
    std::string address;
    std::set<Connection> clients;
    int connectionTimeout;
    void pollNewConnections();
    Connection acceptNewConnection();
    void pollData();
  public:
    Server(
      const Domain::Handle domain,
      const Type::Handle type,
      const Protocol::Handle protocol
    );
    ~Server();
    inline Domain::Handle getDomain() const {
      return this->domain;
    }
    inline Type::Handle getType() const {
      return this->type;
    }
    inline Protocol::Handle getProtocol() const {
      return this->protocol;
    }
    inline const std::string& getAddress() const {
      return this->address;
    }
    inline int getPort() const {
      return this->port;
    }
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
  protected:
    virtual void onStarted() = 0;
    virtual void onNewConnection(Connection& connection) = 0;
    virtual void onDisconnected(Connection& connection) = 0;
    virtual void onData(Connection& connection, const std::string& buffer) = 0;

    friend class Connection;
  };
}

std::ostream& operator<<(std::ostream& os, const Socket::Connection::IO& other);