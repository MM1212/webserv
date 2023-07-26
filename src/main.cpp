#include <HTTP.hpp>
#include <error.h>
#include <stdio.h>
#include <utils/Logger.hpp>
#include <Yaml.hpp>

class IndexRoute : public HTTP::Route {
public:
  IndexRoute()
    : HTTP::Route(HTTP::Methods::GET, "/") {}
  void run(HTTP::Request& req, HTTP::Response& res) const {
    (void)req;
    std::ifstream file;
    file.open("Makefile");
    std::string line, buff;
    while (std::getline(file, line))
      buff.append(line + "\n");
    res.status(200).send(buff);
  }
};

int main(void) {
  YAML::RunTests();
  try {
    HTTP::Server server;
    IndexRoute route;
    server.router.add(route);
    if (!server.listen("0.0.0.0", 8080))
      throw std::runtime_error("Failed to listen");
  }
  catch (const std::exception& e) {
    perror("Error");
    std::cout << e.what() << std::endl;
  }
}