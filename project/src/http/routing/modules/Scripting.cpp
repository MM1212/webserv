#include "http/routing/modules/Scripting.hpp"
#include "http/ServerConfiguration.hpp"

using namespace HTTP;
using namespace HTTP::Routing;

Script::Script(const Route& route, const YAML::Node& node)
  : Module(Types::Script, route, node) {
  this->init();
}

Script::~Script() {
  if (this->state)
    lua_close(this->state);
}

Script::Script(const Script& other) : Module(other) {
  this->init();
}

void Script::init() {
  this->Module::init();
  this->state = luaL_newstate();
  if (!this->state)
    throw std::runtime_error("Failed to create LUA state");
  try {
    luaL_openlibs(this->state);
    const std::ifstream file(this->getFilePath().data());
    if (!file.is_open())
      throw std::runtime_error("Failed to open file: " + std::string(this->getFilePath()));
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (luaL_dostring(this->state, buffer.str().c_str()) != LUA_OK)
      throw std::runtime_error("Failed to load LUA script: " + std::string(lua_tostring(this->state, -1)));
    const uint32_t returnArgs = lua_gettop(this->state);
    if (returnArgs == 0)
      throw std::runtime_error("Script must return a function to handle the request in " + std::string(this->getFilePath()));
    else if (!lua_isfunction(this->state, returnArgs))
      throw std::runtime_error("Expected function to be returned from script in " + std::string(this->getFilePath()) + " but got " + lua_typename(this->state, lua_type(this->state, returnArgs)));
    lua_setglobal(this->state, "__handler");
  }
  catch (const std::exception& e) {
    lua_close(this->state);
    throw std::runtime_error(e.what());
  }
}


bool Script::handle(const Request& req, Response& res) const {
  lua_getglobal(this->state, "__handler");
  if (!lua_isfunction(this->state, -1)) {
    res.status(500).send("Internal Server Error");
    return true;
  }
  if (lua_pcall(this->state, 0, 2, 0) != LUA_OK) {
    res.status(500).send("Internal Server Error: " + std::string(lua_tostring(this->state, -1)));
    return true;
  }
  int returnArgs = lua_gettop(this->state);
  int statusCode = 200;
  if (lua_isnil(this->state, 2)) // 2nd return value
    returnArgs = 1;
  if (returnArgs == 2) {
    if (!lua_isnumber(this->state, 1)) {
      res.status(500).send("Internal Server Error");
      Logger::error << "Expected number to be returned from LUA script but got " << Logger::param(lua_typename(this->state, lua_type(this->state, 1))) << std::endl;
      return true;
    }
    statusCode = lua_tointeger(this->state, 1);

  }
  switch (lua_type(this->state, returnArgs)) {
    case LUA_TSTRING:
      res.status(statusCode).send(lua_tostring(this->state, returnArgs));
      break;
    case LUA_TNUMBER:
      if (lua_isinteger(this->state, returnArgs))
        res.status(statusCode).send(std::to_string(lua_tointeger(this->state, returnArgs)));
      else
        res.status(statusCode).send(std::to_string(lua_tonumber(this->state, returnArgs)));
      break;
    case LUA_TBOOLEAN:
      res.status(statusCode).send(lua_toboolean(this->state, returnArgs) ? "true" : "false");
      break;
    default:
      res.status(500).send("Internal Server Error");
      Logger::error << "Expected string, number or boolean to be returned from LUA script but got " << Logger::param(lua_typename(this->state, lua_type(this->state, returnArgs))) << std::endl;
      return true;
  }
  return true;
}