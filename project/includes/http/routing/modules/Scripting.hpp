/**
 * Scripting.hpp
 * Scripting Module for a Route with LUA.
*/
#pragma once

#include "http/routing/Module.hpp"
#include <lua.hpp>
#include <string_view>

namespace HTTP {
  namespace Routing {
    class Script : public Module {
    public:
      Script(const Route& route, const YAML::Node& node);
      ~Script();
      Script(const Script& other);

      inline Script* clone() const { return new Script(*this); }

      std::string_view getFilePath() const { return this->node["settings"]["path"].as<std::string_view>(); }

      void init();
      bool handle(const Request& req, Response& res) const;
    private:
      lua_State* state;
    };
  }
}