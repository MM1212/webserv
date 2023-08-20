#include "http/routing/modules/CGI.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/Route.hpp"
#include "http/ServerManager.hpp"
#include <Settings.hpp>
#include <utils/misc.hpp>

using namespace HTTP::Routing;

static const Settings* settings = Instance::Get<Settings>();

/*
  6.3.4.  Protocol-Specific Header Fields

  The script MAY return any other header fields that relate to the
  response message defined by the specification for the SERVER_PROTOCOL
  (HTTP/1.0 [1] or HTTP/1.1 [4]).  The server MUST translate the header
  data from the CGI header syntax to the HTTP header syntax if these
  differ.  For example, the character sequence for newline (such as
  UNIX's US-ASCII LF) used by CGI scripts may not be the same as that
  used by HTTP (US-ASCII CR followed by LF).

  The script MUST NOT return any header fields that relate to
  client-side communication issues and could affect the server's
  ability to send the response to the client.  The server MAY remove
  any such header fields returned by the client.  It SHOULD resolve any
  conflicts between header fields returned by the script and header
  fields that it would otherwise send itself.
*/
static const uint32_t forbiddenHeaderFieldsSize = 37;
static const char* forbiddenHeaderFields[] = {
  "Content-Length",
  "Transfer-Encoding",
  "Connection",
  "Upgrade",
  "Host",
  "Expect",
  "TE",
  "Trailer",
  "Proxy-Authorization",
  "Proxy-Authenticate",
  "Proxy-Connection",
  "Keep-Alive",
  "If-Modified-Since",
  "If-Match",
  "If-None-Match",
  "If-Range",
  "If-Unmodified-Since",
  "Range",
  "Referer",
  "User-Agent",
  "Cookie",
  "Set-Cookie",
  "Authorization",
  "Accept-Encoding",
  "Accept-Language",
  "Content-Encoding",
  "Content-Language",
  "Content-Location",
  "Content-MD5",
  "Content-Range",
  "Date",
  "ETag",
  "Expires",
  "Last-Modified",
  "Location",
  "MIME-Version",
  "Retry-After",
  "Server",
  "Vary",
  "Warning",
  "WWW-Authenticate"
};

CGI::CGI(const Route& route, const YAML::Node& node)
  : Module(Types::CGI, route, node) {
  this->init();
}

CGI::~CGI() {}

CGI::CGI(const CGI& other) : Module(other) {
  this->init();
}

const std::string& CGI::getBasePathInfo() const {
  const YAML::Node& settings = this->getSettings();
  if (!settings.has("path_info"))
    return this->getRoot();
  return settings["path_info"].getValue();
}

std::string CGI::getResolvedPath(const Request& req) const {
  const std::string& root = this->getRoot();
  std::string path = req.getPath();
  path.erase(0, this->route.getUri().size());
  path = Utils::resolvePath(2, root.c_str(), path.c_str());
  return path;
}

std::vector<std::string> CGI::resolvePathInfo(const Request& req) const {
  std::string uri = req.getPath();
  std::vector<std::string> result(2);
  // remove route's root uri
  uri.erase(0, this->route.getUri().size());
  // remove route's script
  if (uri[0] == '/')
    uri.erase(0, 1);
  size_t pos = uri.find_first_of('/');
  if (pos == std::string::npos) {
    result[0] = req.getPath();
    result[1] = "/";
    return result;
  }
  result[0] = uri.substr(0, pos);
  result[1] = Utils::resolvePath(1, uri.substr(pos).c_str());
  return result;
}

std::string CGI::resolvePathTranslated(const Request& req, const std::string& pathInfo) const {
  (void)req;
  const std::string& base = this->getBasePathInfo();
  bool isFull = base[0] == '/';
  std::string result = Utils::resolvePath(
    2,
    base.c_str(),
    pathInfo.c_str()
  );
  if (isFull)
    return result;
  return Utils::resolvePath(2, Utils::getCurrentWorkingDirectory().c_str(), result.c_str());
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
  std::string& root = const_cast<std::string&>(settings["root"].getValue());
  if (*root.rbegin() != '/')
    root.append("/");
  root = Utils::expandPath(root);
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
  const bool doesMatch = this->doesFileMatch(path);
  Logger::debug
    << "cgi module triggered for " << Logger::param(path)
    << " | running: " << doesMatch << std::endl;
  if (!doesMatch)
    return this->next(res);
  const Interpreter& interpreter = this->getInterpreterByFile(path);
  return interpreter.run(path, req, res, this);
}

std::vector<std::string> CGI::generateEnvironment(
  const std::string& filePath,
  const CGI::Interpreter* intr,
  const Request& req
) const {
  (void)intr;
  static const ServerManager* serverManager = Instance::Get<ServerManager>();
  const char* const* defaultEnv = serverManager->getEnv();
  std::vector<std::string> env;

  for (size_t i = 0; defaultEnv[i]; i++)
    env.push_back(defaultEnv[i]);
  Headers headers = req.getHeaders();
  const Socket::Server& server = serverManager->getServer(req.getClient().getServerSock());
  const std::map<std::string, std::string>& headersMap = headers.getAll();
  Logger::debug
    << "Headers: " << std::endl
    << headers
    << std::endl;
  if (headers.has("Content-Type"))
    env.push_back(EnvVar("CONTENT_TYPE", headers.get<std::string>("Content-Type")));
  if (req.getBody().size() > 0)
    env.push_back(EnvVar("CONTENT_LENGTH", req.getRawBody().size()));
  headers.remove("Content-Type");
  headers.remove("Content-Length");
  env.push_back(EnvVar("GATEWAY_INTERFACE", "CGI/1.1"));

  const std::vector<std::string> paths = this->resolvePathInfo(req);
  const std::string requestUri = paths[0];
  const std::string pathInfo = paths[1];
  const std::string pathTranslated = this->resolvePathTranslated(req, pathInfo);

  env.push_back(EnvVar("REQUEST_URI", requestUri));
  env.push_back(EnvVar("PATH_INFO", pathInfo));
  env.push_back(EnvVar("PATH_TRANSLATED", pathTranslated));
  env.push_back(EnvVar("QUERY_STRING", req.getQuery()));
  env.push_back(EnvVar("REMOTE_ADDR", req.getClient().getAddress()));
  env.push_back(EnvVar("REMOTE_PORT", Utils::toString(req.getClient().getPort())));
  env.push_back(EnvVar("REQUEST_METHOD", Methods::ToString(req.getMethod())));
  env.push_back(EnvVar("SCRIPT_NAME", Utils::resolvePath(2, this->getRoot().c_str(), Utils::basename(filePath).c_str())));
  env.push_back(EnvVar("SERVER_NAME", server.address));
  env.push_back(EnvVar("SERVER_PORT", Utils::toString(server.port)));
  env.push_back(EnvVar("SERVER_PROTOCOL", req.getProtocol()));
  env.push_back(EnvVar("SERVER_SOFTWARE", settings->get<std::string>("misc.name")));
  for (
    std::map<std::string, std::string>::const_iterator it = headersMap.begin();
    it != headersMap.end();
    it++
    ) {
    std::string key = it->first;
    const std::string& value = it->second;
    env.push_back(EnvVar("HTTP_" + Utils::toUppercase(key), value));
  }
  return env;
}

std::vector<std::string> CGI::generateArgs(
  const std::string& filePath,
  const CGI::Interpreter* intr,
  const Request& req
) const {
  (void)req;
  const YAML::Node& baseArgs = intr->getArgs();
  std::vector<std::string> args(baseArgs.size() + 1);
  args[0] = intr->getPath();
  for (uint64_t i = 0; i < baseArgs.size(); i++) {
    std::string arg = baseArgs[i].getValue();
    uint64_t pos;
    while ((pos = arg.find("$file")) != std::string::npos)
      arg.replace(pos, 5, Utils::basename(filePath));
    args[i + 1] = arg;
  }
  return args;
}

void CGI::handleResponse(Response& res) const {
  std::stringstream packet(res.getBody());
  std::string line;
  Headers& headers = res.getHeaders();
  res.status(200);
  Logger::debug
    << "cgi body: "
    << Logger::param(packet.str()) << std::endl;
  while (std::getline(packet, line)) {
    std::cout << "line: " << line << std::endl;
    if (line.empty())
      break;
    size_t pos = line.find(": ");
    if (pos == std::string::npos)
      throw std::runtime_error("Invalid CGI response header: " + line);
    std::string key = line.substr(0, pos);
    if (key.empty() || HTTP::hasFieldToken(key))
      throw std::runtime_error("Invalid CGI response header: " + line);
    std::string value = line.substr(pos + 2);
    // some overriders
    if (key == "Status") {
      int statusCode = Utils::to<int>(value);
      if (!Utils::isInteger(value, true) || value.size() != 3 || settings->httpStatusCode(statusCode).empty())
        throw std::runtime_error("Invalid CGI response status: " + value);
      res.status(statusCode);
      continue;
    }
    for (size_t i = 0; i < forbiddenHeaderFieldsSize; i++)
      if (std::strncmp(key.c_str(), forbiddenHeaderFields[i], std::strlen(forbiddenHeaderFields[i]) + 1) == 0)
        throw std::runtime_error("Invalid CGI response header: " + line);
    headers.set(key, value);
  }
  if (res.getStatus() / 100 == 2) {
    ByteStream& body = res.getRawBody();
    const int64_t bodySize = res.getRawBody().size() - packet.tellg();
    std::cout << "bodySize: " << bodySize << std::endl;
    body.ignore(packet.tellg());
    res.getHeaders().set("Content-Length", bodySize);
  }
  res.send();
}