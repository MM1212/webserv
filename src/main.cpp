#include <Socket.hpp>
#include <error.h>
#include <stdio.h>

void onNewConnection(const Socket::Connection& con) {
  std::cout << "new connection from " << con.getAddress() << "on port " << con.getPort() << std::endl;
}

void onData(const Socket::Connection& con, const std::string& data) {
  std::cout << "received " << data.length() << " bytes from " << con.getAddress() << "on port " << con.getPort() << std::endl;
  std::cout << "data: " << data << std::endl;
}

int main(void) {
  Socket::Server sock(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP);

  Socket::Handling::NewConnectionHandler newClientListener(&onNewConnection);
  Socket::Handling::RawDataHandler dataListener(&onData);
  sock.dispatcher.on("socket::newConnection", &newClientListener);
  sock.dispatcher.on("socket::data", &dataListener);
  std::cout << "Listening on port " << "8080 " << sock.listen("0.0.0.0", 8080, 10, 0) << std::endl;
  return 0;
}