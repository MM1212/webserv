#pragma once

#include "http/routing/Module.hpp"
#include "http/DirectoryBuilder.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace HTTP {
  namespace Routing {
    class Static : public Module {
    public:
      Static(const Route& route, const YAML::Node& node);
      ~Static();
      Static(const Static& other);

      inline bool supportsExpect() const { return true; }
      inline Static* clone() const { return new Static(*this); }

      const std::string& getRoot() const;
      const std::string getIndex() const;
      const std::string& getRedirection() const;
      bool isDirectoryListingAllowed() const;
      bool ignoreHiddenFiles() const;

      bool handle(const Request& req, Response& res) const;
    private:
      void init();

      std::string getResolvedPath(const Request& req) const;
      std::string buildDirectoryListing(const std::string& path) const;

      bool handleGet(const std::string& path, const Request& req, Response& res) const;
      bool handleUploads(const std::string& path, const Request& req, Response& res) const;
      bool handleDelete(const std::string& path, const Request& req, Response& res) const;
      bool handleFile(const std::string& path, const Request& req, Response& res) const;
      bool clientHasFile(const Request& req, const std::string& filePath, struct stat* st) const;
    };
  }
}