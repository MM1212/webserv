#include "socket/Process.hpp"
#include <utils/Logger.hpp>

using namespace Socket;

Process::Process(
  File& in,
  File& out,
  const Connection& client,
  pid_t id,
  int timeout
)
  : in(&in),
  out(&out),
  client(client),
  id(id),
  timeout(timeout),
  heartbeat(Utils::getCurrentTime()) {
  this->std[0] = in.getFd();
  this->std[1] = out.getFd();
}

Process::Process(const Process& other)
  :
  in(other.in),
  out(other.out),
  client(other.client),
  id(other.id),
  timeout(other.timeout),
  heartbeat(other.heartbeat) {
  this->std[0] = other.std[0];
  this->std[1] = other.std[1];
}

Process::~Process() {}

void Process::kill() {
  if (this->isAlive()) {
    Logger::warning
      << "Killing process " << Logger::param(this->id) << ".." << std::endl;
    ::kill(this->id, SIGKILL);
  }
}

void Process::write(const std::string& buff) {
  this->writeBuffer.append(buff);
}