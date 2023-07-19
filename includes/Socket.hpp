#pragma once

#include <Events.hpp>
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
  private:
    int clientId;
    Server* server;
    std::string address;
    int port;
    uint32_t timeout;
    uint32_t heartbeat;
  public:
    std::string buffer;
    Connection(int clientId, Server* server);
    int getId() const;
    const Server* getServer() const;
    const std::string& getAddress() const;
    int getPort() const;
    bool isAlive() const;
    template <typename T>
    bool send(const T& data, const int flags, const int timeout) const;
    bool disconnect() const;
    void ping();
    bool timedOut() const;

    operator int() const;
  };

  namespace Dispatch {
    class NewConnectionEvent : public Events::Event {
    private:
      const Connection connection;
    public:
      NewConnectionEvent(const Connection& connection);
      ~NewConnectionEvent();
      inline const Connection& getConnection() const;
    };

    template <typename T>
    class DataEvent : public Events::Event {
    private:
      Connection connection;
      T data;
    public:
      DataEvent(const Connection& connection, const T& data)
        : Events::Event("socket::data"), connection(connection), data(data) {}
      inline const Connection& getConnection() const {
        return this->connection;
      }
      inline const T& getData() const {
        return this->data;
      }
    };
  }

  namespace Handling {
    class NewConnectionHandler
      : public Events::EventListener<void (*)(const Connection&)> {
    public:
      NewConnectionHandler(void (*callback)(const Connection&));
      void onEvent(const Events::Event& ev) const;
    };

    class RawDataHandler
      : public Events::EventListener<void (*)(const Connection&, const std::string&)> {
    public:
      RawDataHandler(void (*callback)(const Connection&, const std::string&))
        : Events::EventListener<void (*)(const Connection&, const std::string&)>(callback) {}
      void onEvent(const Events::Event& e) const {
        const Dispatch::DataEvent<std::string>& ev = dynamic_cast<const Dispatch::DataEvent<std::string>&>(e);
        this->handler(ev.getConnection(), ev.getData());
      }
      void onEvent(const Dispatch::DataEvent<std::string>& ev) {
        this->handler(ev.getConnection(), ev.getData());
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
    Events::EventDispatcher dispatcher;
    Server(
      const Domain::Handle domain,
      const Type::Handle type,
      const Protocol::Handle protocol
    );
    ~Server();
    inline Domain::Handle getDomain() const;
    inline Type::Handle getType() const;
    inline Protocol::Handle getProtocol() const;
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
