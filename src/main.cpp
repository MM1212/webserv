#include <HTTP.hpp>
#include <error.h>
#include <stdio.h>
#include <utils/Logger.hpp>
#include <Yaml.hpp>

Utils::Logger log("Test");

// void conStarted(const Socket::Dispatch::StartedEvent& ev) {
//   std::cout << "Listening on port " << ev.getServer().getPort() << std::endl;
// }

// void onNewConnection(const Socket::Dispatch::NewConnectionEvent& ev) {
//   const Socket::Connection& con = ev.getConnection();
//   log.debug("new connection from %s on port %d", con.getAddress().c_str(), con.getPort());
// }

// void onDisconnected(const Socket::Dispatch::DisconnectedEvent& ev) {
//   const Socket::Connection& con = ev.getConnection();
//   log.debug("disconnected from %s on port %d", con.getAddress().c_str(), con.getPort());
// }

// void onData(const Socket::Dispatch::DataEvent<std::string>& ev) {
//   const Socket::Connection& con = ev.getConnection();
//   const std::string& data = ev.getData();
//   log.debug("received %d bytes from %s on port %d", data.length(), con.getAddress().c_str(), con.getPort());
//   std::cout << "data: " << std::endl << data << std::endl;
//   std::stringstream ss;
//   std::ifstream file;
//   file.open("samples/index.html");
//   std::string content, line;
//   while (std::getline(file, line))
//     content.append(line);
//   ss << "HTTP/1.1 200 OK\r\n"
//     << "Content-Type: text/html\r\n"
//     << "Content-Length: " << content.length() << "\r\n"
//     << "Connection: Keep-Alive\r\n"
//     << "\r\n"
//     << content;
//   const std::string resp = ss.str();
//   con.send(resp.c_str(), resp.length(), 0, 0);
//   // con.getServer()->disconnect(con);
// }

// int main(void) {
//   try {
//     Socket::Server sock(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP);

//     sock.dispatcher.on<Socket::Handling::StartedHandler>(conStarted);
//     sock.dispatcher.on<Socket::Handling::NewConnectionHandler>(onNewConnection);
//     sock.dispatcher.on<Socket::Handling::RawDataHandler>(onData);
//     sock.dispatcher.on<Socket::Handling::DisconnectedHandler>(onDisconnected);
//     if (!sock.listen("0.0.0.0", 8080, 10, 0))
//       throw std::runtime_error("Failed to listen");
//   }
//   catch (const std::exception& e) {
//     perror("Error");
//     std::cout << e.what() << std::endl;
//   }
//   return 0;
// }

void indexRoute(HTTP::Request& req, HTTP::Response& res) {
  (void)req;
  std::ifstream file;
  file.open("Makefile");
  std::string line, buff;
  while (std::getline(file, line))
    buff.append(line + "\n");
  res.setStatus(200, "OK").send(buff);
}

int main(void) {
  YAML::RunTests();
  try {
    HTTP::Server server;
    server.routes["/"].insert(std::make_pair(HTTP::Methods::GET, HTTP::Route(
      HTTP::Methods::GET,
      std::string("/"),
      &indexRoute
    )));
    if (!server.listen("0.0.0.0", 8080))
      throw std::runtime_error("Failed to listen");
  }
  catch (const std::exception& e) {
    perror("Error");
    std::cout << e.what() << std::endl;
  }
}