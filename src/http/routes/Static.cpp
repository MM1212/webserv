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

const std::string Static::getIndex() const {
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("index"))
    return this->server->getDefaultRoute()->getIndex();
  return sttc["index"].getValue();
}

const std::string& Static::getRedirection() const {
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("send_to"))
    return this->getRoot();
  return sttc["send_to"].getValue();
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
  this->Route::init();
  const YAML::Node& sttc = this->getNode();
  if (!sttc.has("root") || !sttc["root"].is<YAML::Types::Scalar>())
    throw std::runtime_error("Static route must have a root");
  std::string& root = const_cast<std::string&>(sttc["root"].getValue());
  if (*root.rbegin() != '/')
    root.append("/");
}

std::string Static::getResolvedPath(const Request& req) const {
  const std::string& root = this->getRoot();
  std::string path = req.getPath();
  path.erase(0, this->getPath().size());
  path = Utils::resolvePath(2, root.c_str(), path.c_str());
  return path;
}

std::string Static::buildDirectoryListing(const std::string& path) const {
  const DirectoryBuilder* builder = Instance::Get<DirectoryBuilder>();
  return builder->build(path, *this);
}

void Static::handle(const Request& req, Response& res) const {
  const std::string path = this->getResolvedPath(req);
  Logger::debug
    << "generated path " << Logger::param(path)
    << " based on root " << Logger::param(this->getRoot())
    << " and req path " << Logger::param(req.getPath())
    << std::endl;
  if (!this->isPathValid(path))
    return res.status(403).send();
  switch (req.getMethod()) {
  case Methods::GET:
  case Methods::HEAD:
    if (req.isExpecting())
      return res.status(401).send();
    return this->handleGet(path, req, res);
  case Methods::POST:
  case Methods::PUT:
    return this->handleUploads(path, req, res);
  case Methods::DELETE:
    return this->handleDelete(path, req, res);
  default:
    return res.status(405).send();
  }
}

void Static::handleGet(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return res.status(404).send();
  if (S_ISDIR(st.st_mode)) {
    std::string indexPath = Utils::resolvePath(2, path.c_str(), this->getIndex().c_str());
    if (access(indexPath.c_str(), F_OK) == 0)
      return this->handleFile(indexPath, req, res);
    if (!this->isDirectoryListingAllowed())
      return res.status(404).send();
    if (*req.getPath().rbegin() != '/')
      return res.redirect(req.getPath() + "/", true);
    std::string listing = this->buildDirectoryListing(path);
    return res.status(200).send(listing);
  }
  else if (S_ISREG(st.st_mode))
    return this->handleFile(path, req, res);
  else
    return res.status(403).send();
}

void Static::handleDelete(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return res.status(req.isExpecting() ? 417 : 404).send();
  if (S_ISDIR(st.st_mode))
    return res.status(req.isExpecting() ? 401 : 403).send();
  if (req.isExpecting())
    return res.status(100).send();
  if (std::remove(path.c_str()) != 0) {
    Logger::warning
      << "Got request " << Logger::param(req)
      << " to delete file " << Logger::param(path)
      << "but couldn't: " << Logger::param(strerror(errno));
    return res.status(500).send();
  }
  res.status(204).send();
}

void Static::handleUploads(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  bool exists = stat(path.c_str(), &st) == 0;
  if (exists && req.getMethod() == Methods::POST)
    return res.status(req.isExpecting() ? 417 : 204).send();
  if (req.isExpecting())
    return res.status(100).send();
  std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
  file.write(req.getRawBody().c_str(), req.getRawBody().length());
  if (this->getRedirection() != this->getRoot()) {
    std::string filePath(path);
    filePath.erase(0, this->getRoot().size());
    std::string location = Utils::resolvePath(2, this->getRedirection().c_str(), filePath.c_str());
    res.getHeaders().append("Location", location);
  }
  res.status(!exists ? 201 : 204).send();
}

void Static::handleFile(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return res.status(404).send();
  if (!this->clientHasFile(req, path, &st))
    return res.status(200).sendFile(path);
  res.setupStaticFileHeaders(path, &st);
  return res.status(304).send();
}

bool Static::clientHasFile(const Request& req, const std::string& path, struct stat* st) const {
  if (!st)
    return false;
  std::string etag = Utils::httpETag(path, st->st_mtime, st->st_size);
  std::string lastModified = Utils::getJSONDate(st->st_mtime);
  const Headers& headers = req.getHeaders();
  if (headers.has("If-None-Match") && headers.get<std::string>("If-None-Match") == etag)
    return true;
  if (headers.has("If-Modified-Since") && headers.get<std::string>("If-Modified-Since") == lastModified)
    return true;
  return false;
}