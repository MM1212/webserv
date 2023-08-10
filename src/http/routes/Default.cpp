#include "http/routes/Default.hpp"
#include <utils/Logger.hpp>
#include <Settings.hpp>

using namespace HTTP;
using namespace HTTP::Routes;

static const Settings* settings = Instance::Get<Settings>();

Default::Default(const YAML::Node& node, const ServerConfiguration* server)
  : Route(server, Types::Default, node) {
  this->init();
}

Default::~Default() {}

Default::Default(const Default& other)
  : Route(other) {
  this->init();
}

void Default::handle(const Request& req, Response& res) const {
  (void)req;
  Logger::warning
    << "Default route handler called! This should not happen at all.."
    << std::endl;
  res.status(404).send();
}

void Default::init() {
  if (!this->node.has("error_pages"))
    const_cast<YAML::Node&>(this->node).insert(settings->get<YAML::Node>("http.error_pages"));
}