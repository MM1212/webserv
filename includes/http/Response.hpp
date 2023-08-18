#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>


#include "Methods.hpp"
#include "Headers.hpp"
#include "Request.hpp"
#include "Route.hpp"

namespace HTTP {
  class Request;

  class Response {
  public:

    Response(const Request& req, const Route* route);
    ~Response();
    Response(const Response& other);
    Response& operator=(const Response& other);

    Response& setHeaders(const Headers& headers);
    Response& setBody(const ByteStream& body);
    Response& setBody(const std::string& body);
    Response& setRoute(const Route* route);
    Response& status(uint32_t status);
    Response& status(uint32_t status, const std::string& message);

    Headers& getHeaders() const;
    const std::string getBody() const;
    const ByteStream& getRawBody() const;
    ByteStream& getRawBody();
    uint32_t getStatus() const;
    const std::string& getStatusMessage() const;
    inline const Route* getRoute() const { return this->route; }
    inline const Request& getRequest() const { return *this->req; }

    operator std::string();
    template <typename T>
    T toString() const;
    template <>
    std::string toString() const {
      std::stringstream ss;
      ss << this->getHeader()
        << "\r\n";
      if (this->body.size() > 0 && this->req->getMethod() != Methods::HEAD)
        ss << this->body.toString();
      return ss.str();
    }
    template <>
    ByteStream toString() const {
      ByteStream ss;
      ss.put(this->getHeader());
      ss.put("\r\n");
      if (this->body.size() > 0 && this->req->getMethod() != Methods::HEAD)
        ss.put(this->body);
      return ss;
    }
    void send();
    template <typename T>
    void send(const T& body) {
      std::cout << "sending generic body" << std::endl;
      this->setBody(Utils::toString(body));
      this->send();
    }
    template <>
    void send(const std::string& body) {
      std::cout << "sending string body" << std::endl;
      this->setBody(body);
      this->send();
    }
    template <>
    void send(const ByteStream& body) {
      std::cout << "sending bytestream body" << std::endl;
      this->setBody(body);
      this->send();
    }
    void sendHeader();

    friend std::ostream& operator<<(std::ostream& stream, const Response& response);
  private:
    void _sendChunk(const char* buffer, std::istream& buff, bool last = false);
    void _preSend();
    void _preStream(const std::string& filePath);
  public:
    void setupStaticFileHeaders(const std::string& filePath, struct stat* fileStat = nullptr);

    void stream(std::istream& buff, size_t fileSize = 0);

    void sendFile(const std::string& filePath, bool stream = true, struct stat* fileStat = nullptr);
    void redirect(const std::string& path, bool permanent = true);
  private:
    Response();
    const Request* req;
    Headers headers;
    ByteStream body;
    long statusCode;
    std::string statusMessage;
    bool sent;
    const Route* route;

    void init();
    std::string getHeader() const;
    void afterSend();

  };

}