#pragma once

#include "http/routing/Module.hpp"
#include <utils/misc.hpp>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


namespace HTTP {
  namespace Routing {
    class CGI : public Module {
    public:
      struct EnvVar {
        std::string name;
        std::string value;
        template <typename T>
        EnvVar(const std::string& name, const T& value) : name(name), value(Utils::to<std::string>(value)) {}
        EnvVar(const EnvVar& other) : name(other.name), value(other.value) {}
        inline std::string toString() const { return this->name + "=" + this->value; }
        inline operator std::string() const { return this->toString(); }
      };
    public:
      class Interpreter {
      public:
        Interpreter(const YAML::Node& node);
        ~Interpreter();
        Interpreter(const Interpreter& other);

        inline const std::string& getName() const { return this->node["name"].getValue(); }
        inline const std::string& getPath() const { return this->node["path"].getValue(); }
        inline const YAML::Node& getExtensions() const { return this->node["extensions"]; }
        bool hasExtension(const std::string& ext) const;

        bool run(const std::string& filePath, const Request& req, Response& res, const CGI* cgi) const;
      private:
        void init();
        const YAML::Node& node;
      };
    public:
      CGI(const Route& route, const YAML::Node& node);
      ~CGI();
      CGI(const CGI& other);

      inline bool supportsExpect() const { return true; }
      inline CGI* clone() const { return new CGI(*this); }

      inline const std::string& getRoot() const { return this->getSettings()["root"].getValue(); }
      inline const std::vector<Interpreter>& getInterpreters() const { return this->interpreters; }
      inline bool isExtMapped(const std::string& ext) const { return this->interpreterExtMap.count(ext) > 0; }
      bool doesFileMatch(const std::string& path) const;

      const Interpreter& getInterpreterByExt(const std::string& ext) const;
      const Interpreter& getInterpreterByFile(const std::string& path) const;



      bool handle(const Request& req, Response& res) const;
      void handleResponse(Response& res) const;
      void init();
    private:
      std::string getResolvedPath(const Request& req) const;
      std::vector<std::string> generateEnvironment(const std::string& path, const Interpreter* interpreter, const Request& req) const;
    private:
      std::vector<Interpreter> interpreters;
      std::map<std::string, const Interpreter*> interpreterExtMap;
    };
  }
}