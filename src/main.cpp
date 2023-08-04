#include <Yaml.hpp>
#include <webserv.hpp>
#include <shared.hpp>
#include <utils/Logger.hpp>
#include <utils/misc.hpp>
#include <cstdlib>
#include <http/WebSocket.hpp>
#include <string.h>
#include <Settings.hpp>

struct ListenAddress {
  std::string address;
  int port;
  int maxConnections;

  inline operator std::string() const {
    return this->address + ":" + Utils::toString(this->port);
  }
};

static std::pair<std::string, ListenAddress> extractAddress(const std::string& entry, int maxConnections) {
  std::string::size_type pos = entry.find(':');
  if (pos == std::string::npos)
    return std::make_pair("0.0.0.0:" + entry, (ListenAddress) { "0.0.0.0", std::atoi(entry.c_str()), maxConnections });
  std::vector<std::string> parts = Utils::split(entry, ":");
  return std::make_pair(entry, (ListenAddress) { parts[0], std::atoi(parts[1].c_str()), maxConnections });
}

int main(int ac, char** av) {
  ac--;
  av++;
  YAML::RunTests();
  if (!Instance::Get<Settings>()->isValid())
    return 1;
  try {
    const std::string path = ac == 1 ? av[0] : DEFAULT_CONFIG_PATH;
    const YAML::Node config = YAML::LoadFile(path);
    const YAML::Node& servers = config["servers"];
    std::map<std::string, ListenAddress> listenAddresses;
    for (
      YAML::Node::const_iterator it = servers.begin<YAML::Node::Sequence>();
      it != servers.end<YAML::Node::Sequence>();
      ++it
      ) {
      const YAML::Node& server = *it;
      const YAML::Node& listen = server["listen"];
      int maxConnections = Instance::Get<Settings>()->getMaxConnections();
      if (server.has("max_connections"))
        maxConnections = server["max_connections"].as<int>();
      if (listen.is<YAML::Types::Scalar>())
        listenAddresses.insert(extractAddress(listen.as<std::string>(), maxConnections));
      else if (listen.is<YAML::Types::Sequence>())
        for (
          YAML::Node::const_iterator it = listen.begin<YAML::Node::Sequence>();
          it != listen.end<YAML::Node::Sequence>();
          ++it
          )
          listenAddresses.insert(extractAddress(it->as<std::string>(), maxConnections));
      else
        throw std::runtime_error("Invalid listen address");
    }
    HTTP::WebSocket server;
    Logger::debug
      << "Attempting to bind "
      << Logger::param(listenAddresses.size())
      << " addresses.." << std::endl;
    for (std::map<std::string, ListenAddress>::iterator it = listenAddresses.begin(); it != listenAddresses.end(); it++) {
      const ListenAddress& address = it->second;
      server.bind(Socket::Domain::INET, Socket::Type::TCP, Socket::Protocol::IP, address.address, address.port, address.maxConnections);
      Logger::info << "Listening on " << Logger::param(address.address) << ":" << Logger::param(address.port) << std::endl;
    }
    server.run();
  }
  catch (const std::exception& e) {
    Logger::error
      << "Failed to load config file: "
      << Logger::param(e.what())
      << " | "
      << Logger::param(strerror(errno))
      << std::endl;
    return 1;
  }

  return 0;
}