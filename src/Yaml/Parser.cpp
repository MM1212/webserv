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
  : root("__root__"), current(&this->root) {
  this->root.indent = -1;
  std::ifstream file;

  file.open(path.c_str());
  if (!file.is_open())
    throw std::runtime_error("Could not open file: " + path);
  while (file.peek() != EOF) {
    std::string line;
    std::getline(file, line);
    size_t pos;
    if ((pos = line.find('#')) != std::string::npos)
      line = line.substr(0, pos);
    if (line.empty() || Utils::isWhitespace(line))
      continue;
    while (isblank(line.end()[-1]))
      line = line.substr(0, line.size() - 1);
    this->doc << line << std::endl;
  }
  file.close();
}

Parser::Parser(const std::stringstream& stream)
  : root("__root__"), current(&this->root) {
  this->root.indent = -1;
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

void Parser::handleContext(int indent, const Node* node) {
  if (!this->current)
    return;

  // std::cout << "checking current indent " << *this->current << " " << this->current->indent << " against " << indent << std::endl;
  if (indent > this->current->indent)
  {
    this->stack.push(this->current);
    this->current = const_cast<Node*>(node);
    return;
  }
  while (this->stack.size() > 0 && indent <= this->current->indent) {
    Node* parent = this->stack.top();
    // std::cout << "leaving level " << *this->current << " to " << *parent << std::endl;
    this->current = parent;
    this->stack.pop();
  }
}

void Parser::parse(bool skipIndent, bool skipTokens) {
  if (this->doc.eof() || this->doc.peek() == EOF) return;
  int indent = this->countWhitespace();
  // std::cout << "[" << std::dec << this->doc.peek() << "](" << (char)this->doc.peek() << ")";
  // std::cout << "c indent " << indent << " | " << "container " << *this->current << " indent: " << this->current->indent << " | " << "skipIndent: " << std::boolalpha << skipIndent << " | " << "skipTokens: " << std::boolalpha << skipTokens << std::endl;
  if (this->isEmptyLine())
  {
    this->doc.ignore();
    return this->parse();
  }
  if (!skipIndent && indent <= this->current->indent)
  {
    this->handleContext(indent, NULL);
    return this->parse();
  }
  this->skipWhitespace();
  if (this->doc.peek() == '-' && !skipTokens) {
    this->doc.ignore();
    if (!std::isspace(this->doc.peek())) {
      this->doc.putback('-');
      goto skip_sequence;
    }
    // std::cout << "handling sequence entry @ " << std::dec << this->doc.peek() << std::endl;
    if (!this->current->is<Types::Sequence>()) {
      if (!this->current->is<Types::Null>())
        throw std::runtime_error("Expected a Null, got: " + Types::GetLabel(this->current->type));
      const int cIndent = this->current->indent;
      *this->current = Node::NewSequence(this->current->key);
      this->current->indent = cIndent;
    }
    this->skipWhitespace();
    const Node& node = this->current->insert(Node::NewNull(Utils::toString(this->current->size())));
    const_cast<Node&>(node).indent = indent;
    this->stack.push(this->current);
    this->current = const_cast<Node*>(&node);
    return this->parse(this->doc.peek() != '\n', false);
  }
skip_sequence:
  std::string key = this->retrieveScalar(skipTokens);
  // std::cout << "got key " << key << " " << this->doc.peek() << std::endl;
  if (this->doc.peek() == ':' && !skipTokens) {
    this->doc.ignore();
    if (!std::isspace(this->doc.peek())) {
      this->doc.putback(':');
      goto skip_map;
    }
    // std::cout << "handling map entry @ " << std::dec << this->doc.peek() << std::endl;
    if (!this->current->is<Types::Map>()) {
      if (!this->current->is<Types::Null>())
        throw std::runtime_error("Expected a Null, got: " + Types::GetLabel(this->current->type));
      const int cIndent = this->current->indent;
      *this->current = Node::NewMap(this->current->key);
      this->current->indent = cIndent;
    }
    this->scalar = key;
    this->skipWhitespace();
    // std::cout << "going to deeper level " << std::boolalpha << (!skipIndent && this->doc.peek() == '\n') << std::endl;
    if (!skipIndent && this->doc.peek() == '\n') {
      Node& node = const_cast<Node&>(this->current->insert(Node::NewNull(key)));
      node.indent = skipIndent ? this->current->indent : indent;
      this->stack.push(this->current);
      this->current = const_cast<Node*>(&node);
      return this->parse();
    }
    return this->parse(true, true);
  }
skip_map:
  Node node = Node::NewScalar(this->scalar, key);
  node.indent = skipIndent ? this->current->indent : indent;
  if (this->current->is<Types::Null>())
    *this->current = node;
  else
    this->current->insert(node);
  this->parse();
}

std::string Parser::retrieveScalar(bool ignoreTokens) {
  std::string value;
  this->skipWhitespace();
  char last = 0;
  while (1) {
    if (this->doc.peek() == EOF || this->doc.peek() == '\n')
      break;
    if (!ignoreTokens && (this->doc.peek() == ':' || this->doc.peek() == '-')) {
      char c = this->doc.get();
      char next = this->doc.peek();
      this->doc.putback(c);
      if (std::isspace(next) && (c != '-' || std::isspace(last)))
        break;
    }
    last = this->doc.get();
    value += last;
  }
  return value;
}

Node YAML::LoadFile(const std::string& path) {
  Parser parser(path);
  parser.parse();
  return parser.getRoot();
}