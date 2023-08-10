#include "socket/Parallel.hpp"
#include <utils/Logger.hpp>
#include <algorithm>
#include <Settings.hpp>
#include <utils/misc.hpp>

using Socket::Parallel;
using Socket::Connection;

static Settings* settings = Instance::Get<Settings>();

Parallel::Parallel(int timeout)
  : fileManager(this, &Parallel::onTick), timeout(timeout) {
  try {
    this->fileManager.setTimeout(settings->get<int>("socket.poll_timeout"));
  }
  catch (const std::exception& e) {
    this->fileManager.setTimeout(Utils::getOrderOfMagnitude(timeout) / 4);
  }
}

Parallel::~Parallel() {}

const Socket::Server& Parallel::bind(
  const Domain::Handle domain,
  const Type::Handle type,
  const Protocol::Handle protocol,
  const std::string& address,
  const int port,
  const int backlog
) {
  if (this->hasServer(address, port))
    throw std::runtime_error("Server already bound to address: " + address + ":" + Utils::toString(port));
  int sock = socket(domain, type, protocol);
  if (sock < 0)
    throw std::runtime_error("Failed to create socket");
  sockaddr_in serverAddress;
  serverAddress.sin_family = domain;
  if (address == "*")
    serverAddress.sin_addr.s_addr = INADDR_ANY;
  else
    serverAddress.sin_addr.s_addr = inet_addr(address.c_str());
  serverAddress.sin_port = htons(port);
  // reuse socket
  int reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    throw std::runtime_error("Failed to set socket to reusable");
  if (::bind(sock, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    throw std::runtime_error("Failed to bind socket " + Utils::toString(sock) + " to address " + address + ":" + Utils::toString(port));
  if (listen(sock, backlog) < 0)
    throw std::runtime_error("Failed to listen on socket");
  if (!this->fileManager.add(sock, EPOLLIN | EPOLLET | EPOLLERR))
    throw std::runtime_error("Failed to add socket to file manager");
  Server server(sock, address, port);
  ;
  this->addressesToSock.insert(std::make_pair(address + ":" + Utils::toString(port), sock));
  const Server& ref = this->servers.insert(std::make_pair(sock, server)).first->second;
  return ref;
}

bool Parallel::hasServer(const std::string& address, const int port) const {
  const std::string pair(address + ":" + Utils::toString(port));
  const std::map<std::string, int>::const_iterator it = this->addressesToSock.find(pair);
  if (it == this->addressesToSock.end())
    return false;
  return this->hasServer(it->second);
}

bool Parallel::hasServer(const Server& server) const {
  return this->hasServer(server.sock);
}

bool Parallel::hasServer(const int sock) const {
  return this->servers.count(sock) > 0;
}

bool Parallel::hasClient(const Connection& client) const {
  return this->hasClient(client.getHandle());
}

bool Parallel::hasClient(const std::string& address, const int port) const {
  const std::string pair(address + ":" + Utils::toString(port));
  const std::map<std::string, int>::const_iterator it = this->addressesToSock.find(pair);
  if (it == this->addressesToSock.end())
    return false;
  return this->hasClient(it->second);
}

bool Parallel::hasClient(const int sock) const {
  return this->clients.count(sock) > 0;
}

Socket::Server& Parallel::getServer(const std::string& address, const int port) {
  const std::string pair(address + ":" + Utils::toString(port));
  const std::map<std::string, int>::const_iterator it = this->addressesToSock.find(pair);
  if (it == this->addressesToSock.end())
    throw std::runtime_error("Server not found");
  return this->getServer(it->second);
}

Socket::Server& Parallel::getServer(int sock) {
  std::map<int, Server>::iterator it = this->servers.find(sock);
  if (it == this->servers.end())
    throw std::runtime_error("Server not found");
  return it->second;
}

Connection& Parallel::getClient(const std::string& address, const int port) {
  const std::string pair(address + ":" + Utils::toString(port));
  const std::map<std::string, int>::const_iterator it = this->addressesToSock.find(pair);
  if (it == this->addressesToSock.end())
    throw std::runtime_error("Client not found");
  return this->getClient(it->second);
}

Socket::Connection& Parallel::getClient(const int sock) {
  std::map<int, Connection>::iterator it = this->clients.find(sock);
  if (it == this->clients.end())
    throw std::runtime_error("Client not found");
  return it->second;
}

bool Parallel::kill(int sock) {
  if (!this->hasServer(sock))
    return false;
  this->fileManager.remove(sock, true);
  this->servers.erase(this->getServer(sock));
  std::vector<Connection*> toRemove;
  for (
    std::map<int, Connection>::iterator it = this->clients.begin();
    it != this->clients.end();
    ++it
    ) {
    if (it->second.getServerSock() == sock)
      toRemove.push_back(const_cast<Connection*>(&(it->second)));
  }
  for (
    std::vector<Connection*>::iterator it = toRemove.begin();
    it != toRemove.end();
    ++it
    ) {
    this->disconnect(**it);
  }
  return true;
}

void Parallel::disconnect(const Connection& client) {
  this->disconnect(client.getHandle());
}

void Parallel::disconnect(int client) {
  if (!this->hasClient(client))
    return;
  const Connection con = this->getClient(client);
  const std::string address(con);
  this->onClientDisconnect(con);
  Logger::warning
    << "Client disconnected " << Logger::param(address)
    << " with sock " << Logger::param(client)
    << " on sock " << Logger::param(con.getServerSock()) << std::endl
    << " - timed out: " << Logger::param(con.hasTimedOut()) << std::endl
    << " - alive: " << Logger::param(con.isAlive()) << std::endl;
  this->fileManager.remove(client, true);
  this->addressesToSock.erase(address);
  this->clients.erase(client);
}

void Parallel::run() {
  this->fileManager.start();
}

void Parallel::onTick(const std::vector<File>& changed) {
  if (changed.size() > 0) {
    // Logger::debug
    //   << "New Parallel tick!" << std::endl;
  }
  for (
    std::vector<File>::const_iterator it = changed.begin();
    it != changed.end();
    ++it
    ) {
    const File& file = *it;
    // Logger::debug
    //   << "sock: " << file << " | "
    //   << "is client: " << this->hasClient(file) << " | "
    //   << "is server: " << this->hasServer(file) << " | "
    //   << "readable: " << std::boolalpha << file.isReadable() << " | "
    //   << "writable: " << std::boolalpha << file.isWritable() << ";" << std::endl;
    if (this->hasServer(file))
      this->_onNewConnection(this->getServer(file));
    else if (this->hasClient(file)) {
      const Connection& client = this->getClient(file);
      if (!client.isAlive() || client.hasTimedOut()) {
        // Logger::debug
        //   << "client: " << client << " | "
        //   << "isAlive: " << std::boolalpha << client.isAlive() << " | "
        //   << "hasTimedOut: " << std::boolalpha << client.hasTimedOut() << std::endl;
        this->_onClientDisconnect(client);
        continue;
      }

      if (client.isReadable())
        this->_onClientRead(const_cast<Connection&>(client));
      if (client.isWritable())
        this->_onClientWrite(const_cast<Connection&>(client));
    }
  }
  for (
    std::map<int, Connection>::iterator it = this->clients.begin();
    it != this->clients.end();
    it++
    ) {
    const Connection& client = it->second;
    if (!client.isAlive() || client.hasTimedOut()) {
      // Logger::debug
      //   << "client: " << client << " | "
      //   << "isAlive: " << std::boolalpha << client.isAlive() << " | "
      //   << "hasTimedOut: " << std::boolalpha << client.hasTimedOut() << std::endl;
      this->_onClientDisconnect(client);
    }
  }
}

void Parallel::_onNewConnection(const Server& server) {
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  Logger::debug
    << "Accepting new con on host "
    << Logger::param(server.address) << ":" << Logger::param(server.port)
    << " with sock " << Logger::param(server.sock) << "..."
    << std::endl;
  int clientSock = accept(server.sock, (sockaddr*)&clientAddress, &clientAddressLength);
  if (clientSock < 0)
    throw std::runtime_error("Failed to accept connection");
  if (!this->fileManager.add(clientSock, EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR))
    throw std::runtime_error("Failed to add socket to file manager");
  const File& fileHandle = this->fileManager.get(clientSock);
  Connection client(const_cast<File&>(fileHandle), server.sock, this->timeout);
  const std::string address(client);
  this->addressesToSock.insert(std::make_pair(address, clientSock));
  this->clients.insert(std::make_pair(fileHandle, client));
  Logger::info
    << "Accepted new con from "
    << Logger::param(address)
    << " with sock " << Logger::param(fileHandle)
    << " on sock " << Logger::param(server.sock)
    << std::endl;
  this->onClientConnect(this->getClient(clientSock));
}

void Parallel::_onClientDisconnect(const Connection& client) {
  this->disconnect(client);
}

void Parallel::_onClientRead(Connection& client) {
  static const int bufferSize = settings->get<int>("socket.read_buffer_size");
  std::unique_ptr<char[]> buffer(bufferSize + 1);
  int read = recv(client.getHandle(), buffer, bufferSize, 0);
  if (read <= 0) {
    this->disconnect(client);
    return;
  }
  buffer[read] = '\0';
  client.getReadBuffer() << buffer;
  Logger::debug
    << "got " << Logger::param(read) << " bytes from "
    << Logger::param(static_cast<std::string>(client))
    << std::endl;
    // << "  - " << Logger::param(buffer) << std::endl;
  client.ping();
  // TESTING
  this->onClientRead(client);
}

void Parallel::_onClientWrite(Connection& client) {
  std::string& buffer = client.getWriteBuffer();
  if (buffer.size() == 0) {
    if (client.shouldCloseOnEmptyWriteBuffer())
      this->disconnect(client);
    else
      this->setClientToRead(client);
    return;
  }
  int wrote = send(client.getHandle(), buffer.c_str(), buffer.size(), 0);
  if (wrote < 0) return;
  this->onClientWrite(client, wrote);
  buffer = buffer.substr(wrote);
  client.ping();
}

void Parallel::setClientToRead(Connection& client) {
  if (!this->fileManager.update(client.getHandle(), EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR))
    Logger::error
    << "Could not switch client " << Logger::param(client)
    << " to read-only mode!" << std::endl;
}


void Parallel::setClientToWrite(Connection& client) {
  if (!this->fileManager.update(client.getHandle(), EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR))
    Logger::error
    << "Could not switch client " << Logger::param(client)
    << " to read-only mode!" << std::endl;
}