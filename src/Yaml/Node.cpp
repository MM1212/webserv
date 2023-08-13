#include "Yaml.hpp"
#include <utils/misc.hpp>
#include <iomanip>

using namespace YAML;
using YAML::Node;
using YAML::Types;


std::string Types::GetLabel(Type type) {
  switch (type) {
  case Types::Null:
    return "Null";
  case Types::Scalar:
    return "Scalar";
  case Types::Sequence:
    return "Sequence";
  case Types::Map:
    return "Map";
  case Types::Undefined:
  default:
    return "Undefined";
  }
}

Node::Node()
  : key(""), type(Types::Undefined), indent(0) {
}

Node::Node(const std::string& _key)
  : key(_key), type(Types::Null), indent(0) {
}
Node::Node(const Node& other)
  : key(other.key), value(other.value),
  sequence(other.sequence), map(other.map),
  type(other.type), indent(0) {
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
  return os << node.toString();
}

const Node& Node::get(const std::string& key) const {
  if (!this->is<Types::Map>())
    throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
  if (this->map.count(key) == 0)
    throw std::runtime_error("Key not found: " + key);
  return this->map.at(key);
}

const Node& Node::get(const size_t idx) const {
  if (!this->is<Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
  if (idx >= this->sequence.size())
    throw std::runtime_error("Index out of bounds");
  return this->sequence[idx];
}

Node& Node::get(const std::string& key) {
  if (!this->is<Types::Map>())
    throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
  if (this->map.count(key) == 0)
    this->map[key].setKey(key);
  return this->map[key];
}

Node& Node::get(const size_t idx) {
  if (!this->is<Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
  if (idx > this->size())
    this->sequence[idx] = Node(Utils::toString(idx));
  return this->sequence[idx];
}

const Node& Node::operator[](const std::string& key) const {
  return this->get(key);
}

const Node& Node::operator[](const size_t idx) const {
  return this->get(idx);
}

Node& Node::operator[](const std::string& key) {
  return this->get(key);
}

Node& Node::operator[](const size_t idx) {
  return this->get(idx);
}

std::string Node::toString() const {
  std::stringstream ss;
  ss << "Node(key: " << this->getKey() << ", type: " << Types::GetLabel(this->getType());
  switch (this->getType()) {
  case Types::Scalar:
    ss << ", value: " << this->getValue();
    break;
  case Types::Sequence:
  case Types::Map:
    ss << ", size: " << this->size();
    break;
  case Types::Undefined:
  case Types::Null:
  default:
    break;
  }
  ss << ");";
  return ss.str();
}

Node::operator std::string() const {
  return this->toString();
}

uint32_t Node::size() const {
  switch (this->type) {
  case Types::Map:
    return this->map.size();
  case Types::Sequence:
    return this->sequence.size();
  default:
    return 0;
  }
}

const Node& Node::insert(const Node& node) {
  // std::cout << "inserted " << node << " into " << *this << std::endl;
  switch (this->type) {
  case Types::Map:
    this->map.insert(std::make_pair(node.getKey(), node));
    return this->map.at(node.getKey());
  case Types::Sequence:
    const_cast<Node&>(node).setKey(Utils::toString(this->sequence.size()));
    this->sequence.push_back(node);
    return this->sequence.back();
  default:
    throw std::runtime_error("Expected a container, got: " + Types::GetLabel(this->type));
  }
}

void Node::remove(const std::string& key) {
  if (!this->is<Types::Map>())
    throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
  this->map.erase(key);
}

void Node::remove(size_t idx) {
  if (!this->is<Types::Sequence>())
    throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
  this->remove<Sequence>(this->begin<Sequence>() + idx);
}

std::string Node::expand(uint32_t indent /* = 0 */) const {
  return Node::Expand(*this, indent);
}

Node Node::NewNull(const std::string& key) {
  return Node(key);
}

Node Node::NewSequence(const std::string& key) {
  return Node(key, Sequence());
}

Node Node::NewMap(const std::string& key) {
  return Node(key, Map());
}

std::string Node::Expand(const Node& node, uint32_t indent /* = 0 */) {
  std::stringstream ss;
  ss << std::setw(indent * 2) << std::setfill(' ') << "";
  switch (node.getType()) {
  case Types::Scalar:
    ss << node.getKey() << ": " << node.getValue();
    break;
  case Types::Sequence:
    ss << node.getKey() << ": [" << std::endl;
    for (uint32_t i = 0; i < node.size(); ++i) {
      ss << Node::Expand(node[i], indent + 1) << std::endl;
    }
    ss << std::setw(indent * 2) << std::setfill(' ') << "" << "]";
    break;
  case Types::Map:
    ss << node.getKey() << ": {" << std::endl;
    for (
      Map::const_iterator it = node.begin<Map>();
      it != node.end<Map>();
      ++it
      ) {
      ss << Node::Expand(it->second, indent + 1) << std::endl;
    }
    ss << std::setw(indent * 2) << std::setfill(' ') << "" << "}";
    break;
  default:
    ss << node.getKey() << ": " << Types::GetLabel(node.getType());
    break;
  }
  return ss.str();
}