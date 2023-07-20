#include <Socket.hpp>
#include <error.h>
#include <stdio.h>

void conStarted(const Socket::Dispatch::StartedEvent& ev) {
  std::cout << "Listening on port " << ev.getServer().getPort() << std::endl;
}

void onNewConnection(const Socket::Dispatch::NewConnectionEvent& ev) {
  const Socket::Connection& con = ev.getConnection();
  std::cout << "new connection from " << con.getAddress() << " on port " << con.getPort() << std::endl;
}

void onDisconnected(const Socket::Dispatch::DisconnectedEvent& ev) {
  const Socket::Connection& con = ev.getConnection();
  std::cout << "disconnected from " << con.getAddress() << " on port " << con.getPort() << std::endl;
}

void onData(const Socket::Dispatch::DataEvent<std::string>& ev) {
  const Socket::Connection& con = ev.getConnection();
  const std::string& data = ev.getData();
  std::cout << "received " << data.length() << " bytes from " << con.getAddress() << " on port " << con.getPort() << std::endl;
  std::cout << "data: " << data << std::endl;
}

int main(void) {
  try {
    Socket::Server sock(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP);

    sock.dispatcher.on<Socket::Handling::StartedHandler>(conStarted);
    sock.dispatcher.on<Socket::Handling::NewConnectionHandler>(onNewConnection);
    sock.dispatcher.on<Socket::Handling::RawDataHandler>(onData);
    sock.dispatcher.on<Socket::Handling::DisconnectedHandler>(onDisconnected);
    sock.listen("0.0.0.0", 8080, 10, 0);
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}