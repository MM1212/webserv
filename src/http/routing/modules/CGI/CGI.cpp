#include "http/routing/modules/CGI.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/Route.hpp"
#include <utils/misc.hpp>

using namespace HTTP::Routing;

CGI::CGI(const Route& route, const YAML::Node& node)
  : Module(Types::CGI, route, node) {
  this->init();
}

CGI::~CGI() {}

CGI::CGI(const CGI& other) : Module(other) {
  this->init();
}

std::string CGI::getResolvedPath(const Request& req) const {
  const std::string& root = this->getRoot();
  std::string path = req.getPath();
  path.erase(0, this->route.getUri().size());
  path = Utils::resolvePath(2, root.c_str(), path.c_str());
  return path;
}

bool CGI::doesFileMatch(const std::string& path) const {
  const std::string fileName = Utils::basename(path);
  const std::string fileExt = Utils::getExtension(fileName);
  if (fileExt.empty())
    return false;
  return this->isExtMapped(fileExt);
}

const CGI::Interpreter& CGI::getInterpreterByExt(const std::string& ext) const {
  if (!this->isExtMapped(ext))
    throw std::runtime_error("No interpreter found for extension: " + ext);
  return *this->interpreterExtMap.at(ext);
}

const CGI::Interpreter& CGI::getInterpreterByFile(const std::string& path) const {
  if (!this->doesFileMatch(path))
    throw std::runtime_error("No interpreter found for file: " + path);
  const std::string fileName = Utils::basename(path);
  const std::string fileExt = Utils::getExtension(fileName);
  return *this->interpreterExtMap.at(fileExt);
}

void CGI::init() {
  this->Module::init();
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("root") || settings["root"].getValue().empty())
    throw std::runtime_error("CGI route must have a root");
  if (
    !settings.has("interpreters") ||
    !settings["interpreters"].is<YAML::Types::Sequence>() ||
    settings["interpreters"].size() == 0)
    throw std::runtime_error("CGI route must have at least 1 interpreter");
  for (size_t i = 0; i < settings["interpreters"].size(); i++) {
    this->interpreters.push_back(Interpreter(settings["interpreters"][i]));
    const Interpreter& interpreter = this->interpreters.back();
    const YAML::Node& exts = interpreter.getExtensions();
    for (size_t j = 0; j < exts.size(); j++) {
      const std::string& ext = exts[j].getValue();
      if (this->isExtMapped(ext))
        throw std::runtime_error("CGI route cannot have multiple interpreters for extension: " + ext);
      this->interpreterExtMap[ext] = &interpreter;
    }
  }
}

bool CGI::handle(const Request& req, Response& res) const {
  const std::string& path = this->getResolvedPath(req);
  if (!this->doesFileMatch(path))
    return this->next(res);
  const Interpreter& interpreter = this->getInterpreterByFile(path);
  return interpreter.run(path, req, res, this);
}