#include <HTTP.hpp>
#include <error.h>
#include <stdio.h>
#include <utils/Logger.hpp>
#include <Yaml.hpp>
#include <webserv.hpp>

#define MANUAL_TEST

#ifndef MANUAL_TEST

void parseFile(const YAML::Node& root) {
  using namespace YAML;
  const Node& servers = root["servers"];
  if (!servers.isValid() || !servers.is<Types::Sequence>())
    throw std::runtime_error("Failed to load servers");
  for (
    Node::const_iterator it = servers.begin<Node::Sequence>();
    it != servers.end<Node::Sequence>();
    ++it
    ) {
    const Node& server = *it;
    if (!server.isValid() || !server.is<Types::Map>())
      throw std::runtime_error("Failed to load server");

  }
}

int main(int ac, char** av) {
  ac--;
  av++;
  const std::string configPath(ac > 0 ? av[0] : DEFAULT_CONFIG_PATH);
  try {
    const YAML::Node config = YAML::LoadFile(configPath);
    if (!config.isValid() || !config.is<YAML::Types::Map>())
      throw std::runtime_error("Failed to load config file");
    parseFile(config);
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

#else

class TestRoute : public HTTP::Route {
public:
  TestRoute() : HTTP::Route("/test") {}
  ~TestRoute() {}
  void run(HTTP::Request& req, HTTP::Response& res) const {
    std::cout << req.getBody<std::string>() << std::endl;
    res.status(100).send();
  }
};

int main(void) {
  YAML::RunTests();
  try {
    HTTP::SingleServer server;
    HTTP::FileRoute* route = new HTTP::FileRoute("/", "Makefile");
    if (!server.router.get(*route))
      throw std::runtime_error("Failed to hook file");
    TestRoute* testRoute = new TestRoute();
    server.router.post(*testRoute);
    if (!server.listen("0.0.0.0", 8080))
      throw std::runtime_error("Failed to listen");
  }
  catch (const std::exception& e) {
    perror("Error");
    std::cout << e.what() << std::endl;
  }
}

#endif