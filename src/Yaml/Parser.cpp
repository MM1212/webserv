#include "Yaml.hpp"
#include <fstream>
#include <utils/misc.hpp>

using namespace YAML;

/*
  Simple yaml parser.
  Interprets scalars as strings.
  Supports sequences and maps (and nested).
  Does not support anchors, aliases, tags, etc.
  Supports only comments that start the line with #.
  Does not support multi-line scalars.
  Does not support flow style.
  Does not support directives.

 */

Parser::Parser(const std::string& path)
  : root("root"), current(&this->root) {
  std::ifstream file;

  file.open(path.c_str());
  if (!file.is_open())
    throw std::runtime_error("Could not open file: " + path);
  while (file.peek() != EOF) {
    std::string line;
    std::getline(file, line);
    this->doc << line << std::endl;
  }
  file.close();
}

Parser::Parser(const std::stringstream& stream)
  : root("root"), current(&this->root) {
  this->doc << stream;
}

void Parser::skipWhitespace() {
  while (this->doc.peek() != EOF && isblank(this->doc.peek()))
    this->doc.ignore();
}

int Parser::countWhitespace() {
  std::stringstream ss;
  ss << this->doc.str().substr(this->doc.tellg());
  int count = 0;
  while (ss.peek() != EOF && isblank(ss.peek())) {
    ss.ignore();
    count++;
  }
  return count;
}

bool Parser::isEmptyLine() {
  std::stringstream ss;
  ss << this->doc.str().substr(this->doc.tellg());
  while (ss.peek() != EOF && std::isspace(ss.peek())) {
    if (ss.peek() == '\n')
      return true;
    ss.ignore();
  }
  return false;
}

void Parser::skipComment() {
  while (this->doc.peek() != EOF && this->doc.peek() != '\n')
    this->doc.ignore();
}

void Parser::handleContext(size_t indent, const Node* node) {
  if (!this->current)
    return;

  // std::cout << "checking current indent " << *this->current << " " << this->current->indent << " against " << indent << std::endl;
  if (indent > this->current->indent)
  {
    this->stack.push(this->current);
    this->current = const_cast<Node*>(node);
  }
  else if (this->stack.size() > 0 && indent < this->current->indent)
  {
    Node* parent = this->stack.top();
    // std::cout << "leaving level " << *this->current << " to " << *parent << std::endl;
    this->current = parent;
    this->stack.pop();
  }
}

void Parser::parse(bool skipIndent) {
  (void)skipIndent;
  if (this->doc.peek() == EOF) return;
  size_t indent = this->countWhitespace();
  // std::cout << "[" << std::dec << this->doc.peek() << "] ";
  // std::cout << "c indent " << indent << " | " << "container " << *this->current << " indent: " << this->current->indent << std::endl;
  if (this->doc.peek() == '#')
  {
    this->skipComment();
    return this->parse();
  }
  if (this->isEmptyLine())
  {
    this->doc.ignore();
    return this->parse();
  }
  if (!skipIndent && indent < this->current->indent)
  {
    this->handleContext(indent, NULL);
    return this->parse();
  }
  this->skipWhitespace();
  if (this->doc.peek() == '-') {
    this->doc.ignore();
    // std::cout << "handling sequence entry @ " << std::dec << this->doc.peek() << std::endl;
    if (!this->current->isSequence()) {
      *this->current = Node::NewSequence(this->current->key);
      this->current->indent = indent;
    }
    this->skipWhitespace();
    const Node& node = this->current->insert(Node::NewNull(Utils::toString(this->current->size())));
    this->stack.push(this->current);
    this->current = const_cast<Node*>(&node);
    return this->parse();
  }
  std::string key = this->retrieveScalar();
  // std::cout << "got key " << key << std::endl;
  if (this->doc.peek() == ':') {
    this->doc.ignore();
    // std::cout << "handling map entry @ " << std::dec << this->doc.peek() << std::endl;
    if (!this->current->isMap()) {
      *this->current = Node::NewMap(this->current->key);
      this->current->indent = indent;
    }
    this->scalar = key;
    this->skipWhitespace();
    // std::cout << "going to deeper level " << std::boolalpha << (!skipIndent && this->doc.peek() == '\n') << std::endl;
    if (!skipIndent && this->doc.peek() == '\n') {
      Node& node = const_cast<Node&>(this->current->insert(Node::NewNull(key)));
      this->stack.push(this->current);
      this->current = const_cast<Node*>(&node);
      return this->parse();
    }
    return this->parse(true);
  }
  Node node = Node::NewScalar(this->scalar, key);
  node.indent = indent;
  if (this->current->isNull())
    *this->current = node;
  else
    this->current->insert(node);
  this->parse();
}

std::string Parser::retrieveScalar() {
  std::string value;
  this->skipWhitespace();
  while (
    this->doc.peek() != EOF &&
    this->doc.peek() != '\n' &&
    this->doc.peek() != ':' &&
    this->doc.peek() != '#'
    ) {
    value += this->doc.get();
  }
  return value;
}

Node YAML::LoadFile(const std::string& path) {
  Parser parser(path);
  parser.parse();
  return parser.getRoot();
}