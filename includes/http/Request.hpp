#pragma once

#include <string>
#include <socket/Parallel.hpp>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <exception>

#include "Methods.hpp"
#include "Headers.hpp"

namespace HTTP {

  class Response;
  class Request {
  public:
    Request(
      Socket::Parallel* server,
      Socket::Connection* client,
      Methods::Method method,
      const std::string& path,
      const std::string& body,
      const std::string& protocol,
      const Headers& headers
    );
    ~Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

    const Headers& getHeaders() const;
    Methods::Method getMethod() const;
    const std::string& getPath() const;
    const std::string& getProtocol() const;
    const Socket::Connection& getClient() const;
    Socket::Connection& getClient();
    template <typename T>
    const T getBody() const {
      return static_cast<T>(this->body);
    }
    template <>
    const std::string getBody<std::string>() const {
      return this->body;
    }

    template <typename T>
    T getParam(const std::string& key) const {
      T value;
      if (!std::istringstream(this->params.at(key)) >> value)
        throw std::runtime_error("Could not convert key " + key);
      return value;
    }
    template <>
    std::string getParam<std::string>(const std::string& key) const {
      return this->params.at(key);
    }

    template <>
    bool getParam<bool>(const std::string& key) const {
      bool value;
      std::istringstream ss(this->params.at(key));
      ss >> std::boolalpha >> value;
      if (ss.fail())
        throw std::runtime_error("Could not convert key " + key);
      return value;
    }

    const std::map<std::string, std::string>& getParams() const;

    // headers helpers
    inline const std::string getHost() const {
      return this->getHeaders().get<std::string>("Host");
    }
    inline const std::string getContentType() const {
      return this->getHeaders().get<std::string>("Content-Type");
    }
    inline int getContentLength() const {
      return this->getHeaders().get<int>("Content-Length");
    }

    friend std::ostream& operator<<(std::ostream& os, const Request& request);
  private:
    Headers headers;
    Methods::Method method;
    std::string path;
    std::string body;
    std::string protocol;
    Socket::Parallel* server;
    Socket::Connection* client;
    std::map<std::string, std::string> params;

    Request();
    void parseParams();

    friend class Response;
  };

}