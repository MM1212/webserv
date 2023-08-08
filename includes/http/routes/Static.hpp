#pragma once

#include "http/Route.hpp"

#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>


namespace HTTP {
  class ServerConfiguration;
  namespace Routes {
    class Static : public Route {
    public:
      Static(const YAML::Node& node, const ServerConfiguration* server);
      ~Static();
      Static(const Static& other);

      const std::string& getRoot() const;
      const std::string& getIndex() const;
      bool isDirectoryListingAllowed() const;
      bool isPathValid(const std::string& path) const;


      void handle(const Request& req, Response& res) const;
      virtual inline Static* clone() const {
        return new Static(*this);
      }
      void init();
    private:
      inline const YAML::Node& getNode() const {
        return this->node["static"];
      }
      std::string getResolvedPath(const Request& req) const;
      std::string buildDirectoryListing(const std::string& path) const;
    };
  }
}