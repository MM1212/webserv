#include <Socket.hpp>
#include <cstdlib>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>

namespace HTTP {
  class Server;

  class WebSocket : public Socket::Server {
    WebSocket();
  public:
    WebSocket(HTTP::Server* server);
    ~WebSocket();
    WebSocket(const WebSocket& other);
    WebSocket& operator=(const WebSocket& other);

    void onStarted();
    void onNewConnection(Socket::Connection& connection);
    void onDisconnected(Socket::Connection& connection);
    void onData(Socket::Connection& connection, const std::string& buffer);
  private:
    HTTP::Server* server;
  };
  struct Methods {
    enum Method {
      GET,
      POST,
      PUT,
      DELETE
    };
    static Method fromString(const std::string& str);
  };

  class Server;

  class Headers {
  public:
    Headers();
    ~Headers();
    Headers(const Headers& other);
    Headers& operator=(const Headers& other);
    void clear();
    bool append(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    bool has(const std::string& key) const;
    const std::string& get(const std::string& key) const;

    const std::map<std::string, std::string>& getAll() const;

    operator std::string();
    std::string toString() const;
  private:
    std::map<std::string, std::string> headers;
  };

  std::ostream& operator<<(std::ostream& os, const Headers& headers);

  class Response;
  class Request {
  public:
    Request(
      Server* server,
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
      return static_cast<T>(this->params.at(key));
    }
    template <>
    std::string getParam<std::string>(const std::string& key) const {
      return this->params.at(key);
    }
    template <>
    int getParam<int>(const std::string& key) const {
      return atoi(this->params.at(key).c_str());
    }

    template <>
    bool getParam<bool>(const std::string& key) const {
      return this->params.at(key) == "true";
    }

    const std::map<std::string, std::string>& getParams() const;
  private:
    Headers headers;
    Methods::Method method;
    std::string path;
    std::string body;
    std::string protocol;
    Server* server;
    Socket::Connection* client;
    std::map<std::string, std::string> params;

    Request();
    void parseParams();

    friend class Response;
  };
  class Response {
  public:
    Response(const Request& req);
    ~Response();
    Response(const Response& other);

    Response& setHeaders(const Headers& headers);
    Response& setBody(const std::string& body);
    Response& setStatus(uint32_t status);
    Response& setStatus(uint32_t status, const std::string& message);

    const Headers& getHeaders() const;
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
  private:
    Response();
    Response& operator=(const Response& other);
    const Request& req;
    Headers headers;
    std::string body;
    uint32_t status;
    std::string statusMessage;
    bool sent;

    void init();
    std::string getHeader() const;

  };

  class Route {
  public:
    typedef void (*Handler)(Request& req, Response& res);
    Route(Methods::Method method, const std::string& path, Handler handler);
    Route(const Route& other);
    ~Route();
    Route& operator=(const Route& other);

    operator std::string();
    operator Methods::Method();

    Methods::Method getMethod() const;
    const std::string& getPath() const;
    Handler getHandler() const;
  private:
    Methods::Method method;
    std::string path;
    Handler handler;
    Route();
  };
  class Server {
  public:
    Server();
    ~Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    bool listen(
      const std::string& address,
      const int port
    );

    Events::Dispatcher& getSocketDispatcher();
    // Path -> Method -> Handler
    std::map<std::string, std::map<Methods::Method, Route> > routes;
  private:
    WebSocket socket;
    Utils::Logger log;


    void onStart();
    void onNewConnection(Socket::Connection& connection);
    void onDisconnected(Socket::Connection& connection);
    void onData(Socket::Connection& connection, const std::string& buffer);

    friend class WebSocket;
    friend class Request;
    friend class Response;
  };
}