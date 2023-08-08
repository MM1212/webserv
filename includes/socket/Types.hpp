#pragma once

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>

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

  struct Host {
    int port;
    std::string address;
    Host(int port, const std::string& address) : port(port), address(address) {}
    Host(int port) : port(port), address("*") {}
    Host() : port(0), address("") {}
    Host(const Host& other) : port(other.port), address(other.address) {}
    Host& operator=(const Host& other) {
      if (this == &other) return *this;
      this->port = other.port;
      this->address = other.address;
      return *this;
    }
    bool operator==(const Host& other) const {
      return this->port == other.port && this->address == other.address;
    }
    virtual ~Host() {}
    inline operator std::string() const {
      return this->address + ":" + Utils::toString(this->port);
    }
  };
}