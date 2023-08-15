#include "socket/Process.hpp"

using namespace Socket;

Process::Process(File& in, File& out, pid_t id)
  : in(in), out(out), id(id) {}

Process::Process(const Process& other)
  : in(other.in), out(other.out), id(other.id) {}

Process::~Process() {
  if (this->isAlive())
    this->kill();
  if (!this->in.isClosed())
    close(this->in);
  if (!this->out.isClosed())
    close(this->out);
}

void Process::kill() {
  if (this->isAlive())
    ::kill(this->id, SIGKILL);
}

void Process::write(const std::string& buff) {
  this->writeBuffer.append(buff);
}