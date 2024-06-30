#include "http/routing/modules/Static.hpp"
#include "http/ServerConfiguration.hpp"
#include <utils/misc.hpp>
#include <Settings.hpp>

using namespace HTTP;
using namespace HTTP::Routing;

Static::Static(const Route& route, const YAML::Node& node)
  : Module(Types::Static, route, node) {
  this->init();
}

Static::~Static() {}

Static::Static(const Static& other) : Module(other) {
  this->init();
}

const std::string& Static::getRoot() const {
  const YAML::Node& settings = this->getSettings();
  return settings["root"].getValue();
}

const std::string Static::getIndex() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("index"))
    return this->getServer()->getDefaultRoute()->getIndex();
  return settings["index"].getValue();
}

const std::string& Static::getRedirection() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("send_to"))
    return this->getRoot();
  return settings["send_to"].getValue();
}

bool Static::isDirectoryListingAllowed() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("directory_listing"))
    return this->getServer()->getDefaultRoute()->hasDirectoryListing();
  return settings["directory_listing"].as<bool>();
}

bool Static::ignoreHiddenFiles() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("ignore_hidden"))
    return this->getServer()->getDefaultRoute()->ignoreHiddenFiles();
  return settings["ignore_hidden"].as<bool>();
}

void Static::init() {
  this->Module::init();
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("root") || !settings["root"].is<YAML::Types::Scalar>())
    throw std::runtime_error("Static route must have a root");
  std::string& root = const_cast<std::string&>(settings["root"].getValue());
  if (*root.rbegin() != '/')
    root.append("/");
  root = Utils::expandPath(root);
}

std::string Static::getResolvedPath(const Request& req) const {
  const std::string& root = this->getRoot();
  std::string path = req.getPath();
  path.erase(0, this->route.getUri().size());
  path = Utils::resolvePath(2, root.c_str(), path.c_str());
  return path;
}

std::string Static::buildDirectoryListing(const std::string& path) const {
  const DirectoryBuilder* builder = Instance::Get<DirectoryBuilder>();
  return builder->build(path, *this);
}

bool Static::handle(const Request& req, Response& res) const {
  const std::string path = this->getResolvedPath(req);
  Logger::debug
    << "generated path " << Logger::param(path)
    << " based on root " << Logger::param(this->getRoot())
    << " and req path " << Logger::param(req.getPath())
    << std::newl;
  switch (req.getMethod()) {
  case Methods::GET:
  case Methods::HEAD:
    if (req.isExpecting())
      return this->next(res, 401);
    return this->handleGet(path, req, res);
  case Methods::POST:
  case Methods::PUT:
    return this->handleUploads(path, req, res);
  case Methods::DELETE:
    return this->handleDelete(path, req, res);
  default:
    return this->next(res);
  }
}

bool Static::handleGet(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return this->next(res);
  if (Utils::basename(path).find_first_of('.') == 0 && this->ignoreHiddenFiles())
    return this->next(res);
  if (S_ISDIR(st.st_mode)) {
    std::string indexPath = Utils::resolvePath(2, path.c_str(), this->getIndex().c_str());
    if (access(indexPath.c_str(), F_OK) == 0)
      return this->handleFile(indexPath, req, res);
    if (!this->isDirectoryListingAllowed())
      return this->next(res);
    if (*req.getPath().rbegin() != '/')
      return (res.redirect(req.getPath() + "/", true), true);
    std::string listing = this->buildDirectoryListing(path);
    Logger::debug
      << "Directory listing size: " << Logger::param(listing.size()) << std::newl;
    return (res.status(200).send(listing), true);
  }
  else if (S_ISREG(st.st_mode))
    return this->handleFile(path, req, res);
  else
    return this->next(res, 403);
}

bool Static::handleDelete(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return this->next(res);
  if (S_ISDIR(st.st_mode))
    return this->next(res, 403);
  if (std::remove(path.c_str()) != 0) {
    Logger::warning
      << "Got request " << Logger::param(req)
      << " to delete file " << Logger::param(path)
      << "but couldn't: " << Logger::param(strerror(errno));
    return this->next(res, 500);
  }
  res.status(204).send();
  return true;
}

bool Static::handleUploads(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  bool exists = stat(path.c_str(), &st) == 0;
  if (exists && req.getMethod() == Methods::POST)
    return this->next(res, req.isExpecting() ? 417 : 204);
  if (req.isExpecting())
    return this->next(res, 100);
  std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
  if (!file.is_open() || !file.good())
    return this->next(res, 500);
  Logger::debug
    << "Uploading file " << Logger::param(path)
    << " with size " << Logger::param(req.getRawBody().size())
    << std::newl;
  file.write(reinterpret_cast<const char*>(req.getRawBody().data()), req.getRawBody().size());
  file.close();
  if (this->getRedirection() != this->getRoot()) {
    std::string filePath(path);
    filePath.erase(0, this->getRoot().size());
    std::string location = Utils::resolvePath(2, this->getRedirection().c_str(), filePath.c_str());
    res.getHeaders().append("Location", Utils::encodeURIComponent(location));
  }
  res.status(!exists ? 201 : 204).send();
  return true;
}

bool Static::handleFile(const std::string& path, const Request& req, Response& res) const {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return this->next(res);
  if (!this->clientHasFile(req, path, &st))
    return (res.status(200).sendFile(path, true, &st), true);
  res.setupStaticFileHeaders(path, &st);
  return this->next(res, 304);
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