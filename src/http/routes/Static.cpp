#include "http/routes/Static.hpp"
#include "http/ServerConfiguration.hpp"
#include <utils/misc.hpp>
#include <Settings.hpp>


using namespace HTTP;
using namespace HTTP::Routes;

static const Settings* settings = Instance::Get<Settings>();

Static::Static(const YAML::Node& node, const ServerConfiguration* server)
  : Route(server, Types::Static, node) {
  this->init();
}

Static::~Static() {}

Static::Static(const Static& other)
  : Route(other) {
  this->init();
}

const std::string& Static::getRoot() const {
  const YAML::Node& sttc = this->getNode();
  return sttc["root"].getValue();
}

const std::string& Static::getIndex() const {
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("index"))
    return this->server->getDefaultRoute()->getIndex();
  return sttc["index"].getValue();
}

bool Static::isDirectoryListingAllowed() const {
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("directory_listing"))
    return this->server->getDefaultRoute()->hasDirectoryListing();
  return sttc["directory_listing"].as<bool>();
}

bool Static::isPathValid(const std::string& path) const {
  std::vector<std::string> parts = Utils::split(path, "/");
  int rootCount = 0;
  for (
    std::vector<std::string>::iterator it = parts.begin();
    it != parts.end();
    ++it
    ) {
    if (*it == "..")
      rootCount--;
    else if (*it != ".")
      rootCount++;
  }
  return rootCount >= 0;
}

void Static::init() {
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("root") || !sttc["root"].is<YAML::Types::Scalar>())
    throw std::runtime_error("Static route must have a root");
}

std::string Static::getResolvedPath(const Request& req) const {
  const std::string& root = this->getRoot();
  std::string path = root + req.getPath();
  size_t pos;
  while ((pos = path.find("//")) != std::string::npos)
    path = path.replace(pos, 2, "/");
  if (path[path.size() - 1] == '/')
    path += this->getIndex();
  return path;
}

std::string Static::buildDirectoryListing(const std::string& path) const {
  std::string listing;
  DIR* dir = opendir(path.c_str());
  if (!dir)
    return listing;
  struct dirent* ent;
  while ((ent = readdir(dir))) {
    if (ent->d_name[0] == '.')
      continue;
    listing += "<a href=\"";
    listing += ent->d_name;
    listing += "\">";
    listing += ent->d_name;
    listing += "</a><br />";
  }
  closedir(dir);
  return listing;
}

void Static::handle(const Request& req, Response& res) const {
  const std::string path = this->getResolvedPath(req);
  if (!this->isPathValid(path))
    return res.status(403).send();
  bool doesPathResolve = access(path.c_str(), F_OK) == 0;
  if (!doesPathResolve) {
    if (this->isDirectoryListingAllowed()) {
      res.getHeaders().set("Content-Type", settings->httpMimeType("html"));
      return res.status(200).send(this->buildDirectoryListing(path));
    }
    return res.status(404).send();
  }
  try {
    res.status(200).sendFile(path);
  }
  catch (const std::exception& e) {
    res.status(403).send();
    Logger::error
      << "Could not handle request: " << e.what() << std::endl;
  }
}