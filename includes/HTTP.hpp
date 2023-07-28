#include <Socket.hpp>
#include <cstdlib>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <Yaml.hpp>

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
      DELETE,
      HEAD
    };
    static Method fromString(const std::string& str);
  };

  class Headers {
  public:
    Headers();
    ~Headers();
    Headers(const Headers& other);
    Headers& operator=(const Headers& other);
    void clear();
    bool append(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    void remove(const std::string& key);
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

  std::ostream& operator<<(std::ostream& os, const Request& req);
  class Response {
  public:

    class StreamException : std::exception {
    public:
      const char* what() const throw() {
        return "Stream error";
      }
    };

    Response(const Request& req);
    ~Response();
    Response(const Response& other);

    Response& setHeaders(const Headers& headers);
    Response& setBody(const std::string& body);
    Response& status(uint32_t status);
    Response& status(uint32_t status, const std::string& message);

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
    void sendHeader();

  private:
    bool _sendChunk(const char* buffer, std::istream& buff, bool last = false);
    void _preStream();
  public:
    void stream(std::istream& buff);
    inline void stream(const std::string& filePath) {
      std::ifstream file(filePath.c_str());
      if (!file.is_open())
        throw StreamException();
      this->stream(reinterpret_cast<std::fstream&>(file));
      file.close();
    }

    void sendFile(const std::string& filePath);
    void redirect(const std::string& path);
  private:
    Response();
    Response& operator=(const Response& other);
    const Request& req;
    Headers headers;
    std::string body;
    long statusCode;
    std::string statusMessage;
    bool sent;

    void init();
    std::string getHeader() const;
  };

  class Router;

  struct Routes {
    enum Type {
      Normal,
      File,
      Directory,
      Upload
    };
    static std::string GetLabel(const Type type);
  };

  class Route {
  public:
    Route(const std::string& path);
    Route(const Route& other);
    virtual ~Route();
    Route& operator=(const Route& other);

    operator std::string();

    const std::string& getPath() const;
    Routes::Type getType() const;
    void setPath(const std::string& path);
    void setType(const Routes::Type type);
  private:
    std::string path;
    Routes::Type type;
    Route();
    virtual void run(Request& req, Response& res) const = 0;

    friend class Router;
  };

  class FileRoute : public Route {
  public:
    FileRoute(const std::string& path, const std::string& filePath);
    FileRoute(const FileRoute& other);
    virtual ~FileRoute();
    FileRoute& operator=(const FileRoute& other);
  private:
    std::string filePath;
    virtual void run(Request& req, Response& res) const;
    void sendHeaders(Response& res) const;
  };

  class Router {
  public:
    class NotFoundException : std::exception {
    public:
      const char* what() const throw() {
        return "Route not found";
      }
    };
  public:
    Router();
    ~Router();
    Router(const Router& other);
    Router& operator=(const Router& other);

    const Route& search(const std::string& path, Methods::Method method) const;
    bool has(const std::string& path, const Methods::Method method) const;
    bool has(const std::string& path) const;
    bool has(const Route& route, const Methods::Method method) const;
    bool add(const Route& route, const Methods::Method method);
    template <typename T>
    bool add() {
      const T route;
      return this->add(route);
    }

    bool get(const Route& route);
    bool post(const Route& route);
    bool put(const Route& route);
    bool del(const Route& route);
    bool head(const Route& route);

    bool hookFile(const FileRoute& route);
  private:
    void run(Request& req, Response& res) const;

    std::map<std::string, std::map<Methods::Method, Route*> > routes;

    friend class Server;
  };

  class ParallelServer;

  class Server {
  public:
    Server();
    virtual ~Server();
    Server(const Server& other);
    Server& operator=(const Server& other);

    Router router;
  private:
    Utils::Logger log;
    YAML::Node statusCodes;


    virtual void onStart() = 0;
    void onNewConnection(Socket::Connection& connection);
    void onDisconnected(Socket::Connection& connection);
    void onData(Socket::Connection& connection, const std::string& buffer);

    void loadStatusCodes();

    friend class WebSocket;
    friend class Request;
    friend class Response;
    friend class ParallelServer;
  };

  class SingleServer : public Server {
  private:
    WebSocket socket;
  public:
    SingleServer();
    ~SingleServer();
    SingleServer(const SingleServer& other);
    SingleServer& operator=(const SingleServer& other);
    bool listen(
      const std::string& address,
      const int port
    );
    void onStart();
  };

  class ParallelServer {
  private:
    Socket::ParallelServer sockets;
    std::vector<Server*> servers;
  public:
    ParallelServer();
    ~ParallelServer();
    ParallelServer(const ParallelServer& other);
    ParallelServer& operator=(const ParallelServer& other);
    bool add(Server* server, const std::string& address, const int port, const int backlog);
    template <typename T>
    Server* create(
      const std::string& address,
      const int port,
      const int backlog = 128
    ) {
      Server* server = new T();
      if (!server)
        return nullptr;
      if (!this->add(server, address, port, backlog))
      {
        delete server;
        return nullptr;
      }
      return server;
    }
    inline bool isRunning() const {
      return this->sockets.isRunning();
    }
    void start(
      const int timeout = 0
    );
    void stop();
  };

}
