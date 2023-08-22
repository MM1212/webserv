#include "http/routing/modules/CGI.hpp"
#include "http/Request.hpp"
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <http/ServerManager.hpp>

using namespace HTTP::Routing;

static HTTP::ServerManager* serverManager = Instance::Get<HTTP::ServerManager>();

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
  if (
    !this->node.has("args") ||
    !this->node["args"].is<YAML::Types::Sequence>() ||
    this->node["args"].size() == 0)
    throw std::runtime_error("CGI interpreter must have at least 1 arg (preferably with $file expander)");

}

bool CGI::Interpreter::run(const std::string& filePath, const Request& req, Response& res, const CGI* cgi) const {
  std::string execPath = this->getPath();
  if (execPath[0] != '/')
    execPath = Utils::resolvePath(2, Utils::getCurrentWorkingDirectory().c_str(), execPath.c_str());
  if (access(execPath.c_str(), F_OK | X_OK) == -1) {
    Logger::error
      << "Attempted to execute a cgi script with an interpreter that doesn't exist: "
      << Logger::param(execPath) << ". Skipping CGI module.." << std::endl;
    return cgi->next(res);
  }
  Logger::debug
    << "Preparing cgi execution for script: "
    << Logger::param(filePath)
    << " & interpreter: " << Logger::param(this->getName()) << std::endl;
  int stdinput[2];
  int stdoutput[2];

  if (pipe(stdinput) < -1) {
    Logger::error
      << "Failed to open a pipe for cgi req "
      << Logger::param(req) << std::endl;
    return cgi->next(res, 500);
  }
  if (pipe(stdoutput) < -1) {
    Logger::error
      << "Failed to open a pipe for cgi req "
      << Logger::param(req) << std::endl;
    return cgi->next(res, 500);
  }
  std::vector<std::string> env;
  cgi->generateEnvironment(env, req);
  std::vector<std::string> baseArgs = cgi->generateArgs(filePath, this, req);
  Logger::debug
    << "Spawning cgi process " << Logger::param(execPath)
    << " for script " << Logger::param(Utils::basename(filePath))
    << " with args: " << Logger::param(Utils::strJoin(baseArgs)) << std::endl
    << " and env size of : " << Logger::param(env.size()) << std::endl;
  pid_t pid = fork();
  if (pid == -1) {
    Logger::error
      << "Failed to fork for cgi req "
      << Logger::param(req) << std::endl;
    return cgi->next(res, 500);
  }
  int std[2];
  std[0] = stdoutput[0];
  std[1] = stdinput[1];
  if (pid != 0) {
    close(stdinput[0]);
    close(stdoutput[1]);
    serverManager->trackCGIResponse(pid, std, res);
  }
  else {
    std::signal(SIGINT, SIG_DFL);
    std::signal(SIGPIPE, SIG_DFL);
    std::string scriptDirName = Utils::dirname(
      Utils::resolvePath(2,
        Utils::getCurrentWorkingDirectory().c_str(),
        filePath.c_str()
      )
    );
    chdir(scriptDirName.c_str());
    std::vector<char*> envp(env.size() + 1);
    for (size_t i = 0; i < env.size(); i++)
      envp[i] = const_cast<char*>(env[i].c_str());
    std::vector<char*> args(baseArgs.size() + 1);
    for (size_t i = 0; i < baseArgs.size(); i++)
      args[i] = const_cast<char*>(baseArgs[i].c_str());
    args[baseArgs.size()] = nullptr;
    int dups[2];
    dups[0] = dup2(stdinput[0], STDIN_FILENO);
    dups[1] = dup2(stdoutput[1], STDOUT_FILENO);
    close(stdinput[0]);
    close(stdinput[1]);
    close(stdoutput[0]);
    close(stdoutput[1]);
    execve(execPath.c_str(), args.data(), envp.data());
    close(dups[0]);
    close(dups[1]);
    std::perror("execve");
    std::exit(1);
  }
  return true;
}