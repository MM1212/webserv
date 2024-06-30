/**
 * Request.hpp
 * Request class for HTTP.
 * It stores an HTTP Request.
 * Has a bunch of helper methods.
 * Also stores the client socket.
 * The body is stored as a ByteStream.
*/
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
    struct File {
      std::string name;
      std::string path;
      std::string data;
      File(const std::string& name, const std::string& path, const std::string& data) :
        name(name), path(path), data(data) {}
    };
  public:
    Request(
      Socket::Parallel* server,
      Socket::Connection* client,
      Methods::Method method,
      const std::string& path,
      const ByteStream& body,
      const std::string& protocol,
      const Headers& headers,
      const std::map<std::string, std::string>& params,
      const std::vector<File>& files
    );
    ~Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

    const Headers& getHeaders() const;
    Methods::Method getMethod() const;
    const std::string& getPath() const;
    const std::string getUri() const;
    const std::string& getProtocol() const;
    const Socket::Connection& getClient() const;
    Socket::Connection& getClient();
    const Socket::Server& getServer() const;
    Socket::Server& getServer();
    inline const ByteStream& getRawBody() const {
      return this->body;
    }
    inline const std::string getBody() const {
      return this->body.toString();
    }

    template <typename T>
    T getParam(const std::string& key) const {
      if constexpr (std::is_same_v<T, std::string>)
        return this->_getParamString(key);
      else if constexpr (std::is_same_v<T, bool>)
        return this->_getParamBool(key);
      T value;
      if (!std::istringstream(this->params.at(key)) >> value)
        throw std::runtime_error("Could not convert key " + key);
      return value;
    }
  private:
    std::string _getParamString(const std::string& key) const {
      return this->params.at(key);
    }

    bool _getParamBool(const std::string& key) const {
      bool value;
      std::istringstream ss(this->params.at(key));
      ss >> std::boolalpha >> value;
      if (ss.fail())
        throw std::runtime_error("Could not convert key " + key);
      return value;
    }
  public:
    const std::map<std::string, std::string>& getParams() const;
    const std::string getQuery() const;
    inline const std::vector<File>& getFiles() const {
      return this->files;
    }
    // headers helpers
    inline const std::string getHost() const {
      return this->getHeaders().get<std::string>("Host");
    }
    inline const std::string getContentType() const {
      return this->getHeaders().get<std::string>("Content-Type");
    }
    inline int getContentLength() const {
      return this->getHeaders().has("Content-Length") ? this->getHeaders().get<int>("Content-Length") : 0;
    }

    inline bool isExpecting() const {
      return this->getHeaders().has("Expect") && this->getContentLength() > 0;
    }

    friend std::ostream& operator<<(std::ostream& os, const Request& request);
  protected:
    Headers headers;
    Methods::Method method;
    std::string path;
    ByteStream body;
    std::string protocol;
    Socket::Parallel* server;
    Socket::Connection* client;
    std::map<std::string, std::string> params;
    std::vector<File> files;

    Request();
    void parseParams();

    friend class Response;
  };

}