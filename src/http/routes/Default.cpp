#include "http/routes/Default.hpp"
#include <utils/Logger.hpp>
#include <Settings.hpp>

using namespace HTTP;
using namespace HTTP::Routes;

static const Settings* settings = Instance::Get<Settings>();

Default::Default(const YAML::Node& node, const ServerConfiguration* server)
  : Route(server, node) {
  this->init(false);
}

Default::~Default() {}

Default::Default(const Default& other)
  : Route(other) {
  this->init(false);
}

void Default::init(bool injectMethods /* = true */) {
  YAML::Node& root = const_cast<YAML::Node&>(this->getSettings());
  if (!root.has("error_pages"))
    root.insert(settings->get<YAML::Node>("http.error_pages"));
  if (!root.has("methods"))
    root.insert(YAML::Node::NewSequence("methods"));
  root.insert(YAML::Node::NewScalar("uri", "__default"));
  this->Route::init(injectMethods);
}