#pragma once

#include "http/Route.hpp"
#include "http/DirectoryBuilder.hpp"

#include <string>
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
      const std::string getIndex() const;
      const std::string& getRedirection() const;
      bool isDirectoryListingAllowed() const;
      bool isPathValid(const std::string& path) const;


      void handle(const Request& req, Response& res) const;
      virtual inline Static* clone() const {
        return new Static(*this);
      }
      bool supportsCascade() const { return true; }
      bool supportsExpect() const { return true; }
      void init();
    private:
      inline const YAML::Node& getNode() const {
        return this->node["static"];
      }
      std::string getResolvedPath(const Request& req) const;
      std::string buildDirectoryListing(const std::string& path) const;

      void handleGet(const std::string& path, const Request& req, Response& res) const;
      void handleUploads(const std::string& path, const Request& req, Response& res) const;
      void handleDelete(const std::string& path, const Request& req, Response& res) const;
      void handleFile(const std::string& path, const Request& req, Response& res) const;
      bool clientHasFile(const Request& req, const std::string& filePath, struct stat* st) const;
    };
  }
}