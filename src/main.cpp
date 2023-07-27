#include <HTTP.hpp>
#include <error.h>
#include <stdio.h>
#include <utils/Logger.hpp>
#include <Yaml.hpp>

int main(void) {
  YAML::RunTests();
  try {
    HTTP::Server server;
    HTTP::FileRoute route(HTTP::Methods::GET, "/", "Makefile");
    if (!server.router.hookFile(route))
      throw std::runtime_error("Failed to hook file");
    if (!server.listen("0.0.0.0", 8080))
      throw std::runtime_error("Failed to listen");
  }
  catch (const std::exception& e) {
    perror("Error");
    std::cout << e.what() << std::endl;
  }
}