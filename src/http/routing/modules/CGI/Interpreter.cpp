#include "http/routing/modules/CGI.hpp"
#include "http/Request.hpp"
#include <utils/misc.hpp>
#include <utils/Logger.hpp>

using namespace HTTP::Routing;

CGI::Interpreter::Interpreter(const YAML::Node& node) : node(node) {
  this->init();
}

CGI::Interpreter::~Interpreter() {}

CGI::Interpreter::Interpreter(const Interpreter& other) : node(other.node) {
  this->init();
}

bool CGI::Interpreter::hasExtension(const std::string& extVal) const {
  const YAML::Node& exts = this->getExtensions();
  for (size_t i = 0; i < exts.size(); i++) {
    const YAML::Node& ext = exts[i];
    if (ext.getValue() == extVal)
      return true;
  }
  return false;
}

void CGI::Interpreter::init() {
  if (!this->node.has("name") || this->node["name"].getValue().empty())
    throw std::runtime_error("CGI interpreter must have a name");
  if (!this->node.has("path") || this->node["path"].getValue().empty())
    throw std::runtime_error("CGI interpreter must have a path");
  if (
    !this->node.has("extensions") ||
    !this->node["extensions"].is<YAML::Types::Sequence>() ||
    this->node["extensions"].size() == 0)
    throw std::runtime_error("CGI interpreter must have at least 1 extension");
}

// TODO:
//  - tell ServerManager of our newly created process
//  - add process stdin to fileManager (and write to its writeBuffer the body)
//  - add to websocket pendingCGI Processes
//  - interpret the response from the process pipe
//  - send the response to the client

// start the proccess execPath with fork & execve
bool CGI::Interpreter::run(const std::string& filePath, const Request& req, Response& res, const CGI* cgi) const {
  const std::string& execPath = this->getPath();
  if (access(execPath.c_str(), F_OK | X_OK) == -1) {
    Logger::error
      << "Attempted to execute a cgi script with an interpreter that doesn't exist: "
      << Logger::param(execPath) << ". Skipping CGI module.." << std::endl;
    return cgi->next(res);
  }
  int std[2];

  if (pipe(std) < -1) {
    Logger::error
      << "Failed to open a pipe for cgi req "
      << Logger::param(req) << std::endl;
    return cgi->next(res);
  }
  // char** env = cgi->generateEnvironment(req);
  pid_t pid = fork();
  if (pid == -1) {
    Logger::error
      << "Failed to fork for cgi req "
      << Logger::param(req) << std::endl;
    return cgi->next(res);
  }
  write(std[1], req.getRawBody().c_str(), req.getRawBody().size());
  if (pid == 0) {
    dup2(std[0], STDIN_FILENO);
    close(std[0]);
    dup2(std[1], STDOUT_FILENO);
    close(std[1]);

    char** env = cgi->generateEnvironment(filePath, this, req);
    char** args = new char* [2];
    args[0] = const_cast<char*>(execPath.c_str());
    args[1] = nullptr;
    execve(execPath.c_str(), args, env);
    delete[] args;
    delete[] env;
  }
  close(std[0]);
  close(std[1]);
}