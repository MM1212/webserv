/**
 * ServerConfiguration.hpp
 * The HTTP::ServerConfiguration class is used to store the configuration of a server.
 * It is used by the HTTP::ServerManager class.
 * It stores all the routes, hosts, names that the server will use.
 * When a HTTP Request is received, it will choose the route that matches the request.
 * If not route is found, it will send 404 Not Found.
 * If a route is found but the default protections are not met, it will send the appropriate error code.
 * If the route is found and the default protections are met, it will call the handle method on the Route.
*/
#pragma once

#include <string>
#include <Yaml.hpp>
#include <utils/misc.hpp>
#include <socket/Types.hpp>
#include "http/Methods.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/Route.hpp"
#include "http/routes/Default.hpp"

#include <map>
#include <vector>
#include <iostream>

namespace HTTP {
  class ServerConfiguration {
  public:
    ServerConfiguration(const YAML::Node& config);
    ~ServerConfiguration();
    ServerConfiguration(const ServerConfiguration& other);

    bool match(const Request& req) const;
    const std::vector<Socket::Host>& getHosts() const;
    const std::vector<std::string>& getNames() const;
    bool hasName(const std::string& name) const;
    bool hasHost(const Socket::Host& host) const;
    bool isDefaultHost() const;
    void setAsDefaultHost(bool state = true);
    inline const YAML::Node& getSettings() const {
      return this->config["settings"];
    }
    int getMaxConnections() const;

    const Route* getRoute(const std::string& path) const;

    // will search for the nearest route, i.e. /a/b/c will match /a/b route
    const Route* getNearestRoute(const std::string& path) const;
    const Routes::Default* getDefaultRoute() const;

    void handleRequest(const Request& req, Response& res) const;
    void init();

    friend std::ostream& operator<<(std::ostream& os, const ServerConfiguration& server);
  private:
    void initHosts();
    void parseHostNode(const YAML::Node& node);
    void initNames();
    void initRoutes();
    void addRoute(const std::string& path, const Route* route);
    void validate();
    const YAML::Node& config;
    std::vector<Socket::Host> hosts;
    std::vector<std::string> names;
    bool defaultHost;
    Routes::Default* defaultRoute;
    std::map<std::string, const Route*> routes;
  };
}