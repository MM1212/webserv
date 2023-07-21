#include <Socket.hpp>

namespace HTTP {
  struct Methods {
    enum Method {
      GET,
      POST,
      PUT,
      DELETE
    };
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

  class Request {
  public:
    ~Request();
    Request(const Request& other);
    Request& operator=(const Request& other);

    const Headers& getHeaders() const;
    Methods::Method getMethod() const;
    const std::string& getPath() const;
    const std::string& getBody() const;
  private:
    Headers headers;
    Methods::Method method;
    std::string path;
    std::string body;
    Server& server;

    Request();
  };
  class Response {};

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
    Server(const Server& other);
    Server& operator=(const Server& other);
  private:
    Socket::Server& socket;
  };
}