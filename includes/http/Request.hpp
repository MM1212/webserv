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
      const std::string& body,
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
    const std::string& getProtocol() const;
    const Socket::Connection& getClient() const;
    Socket::Connection& getClient();
    template <typename T>
    const T getBody() const {
      return static_cast<T>(this->body);
    }
    inline const std::string& getRawBody() const {
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
    std::string body;
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