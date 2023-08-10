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

    Response& setHeaders(const Headers& headers);
    Response& setBody(const std::string& body);
    Response& setRoute(const Route* route);
    Response& status(uint32_t status);
    Response& status(uint32_t status, const std::string& message);

    Headers& getHeaders() const;
    const std::string& getBody() const;
    uint32_t getStatus() const;
    const std::string& getStatusMessage() const;

    operator std::string();
    std::string toString() const;
    void send();
    template <typename T>
    void send(const T& body) {
      this->setBody(Utils::toString(body));
      this->send();
    }
    void sendHeader();

    friend std::ostream& operator<<(std::ostream& stream, const Response& response);
  private:
    void _sendChunk(const char* buffer, std::istream& buff, bool last = false);
    void _preSend();
    void _preStream(const std::string& filePath);
  public:
    void stream(std::istream& buff);

    void sendFile(const std::string& filePath, bool stream = true);
    void redirect(const std::string& path, bool permanent = true);
  private:
    Response();
    const Request& req;
    Headers headers;
    std::string body;
    long statusCode;
    std::string statusMessage;
    bool sent;
    const Route* route;

    void init();
    std::string getHeader() const;
    void afterSend();

  };

}