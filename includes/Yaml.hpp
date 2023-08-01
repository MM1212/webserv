#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <shared.hpp>
#include <typeinfo>

namespace YAML {
  class Node;

  struct Types {
    enum Type {
      Undefined,
      Null,
      Scalar,
      Sequence,
      Map
    };
    static std::string GetLabel(Type type);
  };

  class Parser;

  class Node {
  public:
    typedef std::vector<Node> Sequence;
    typedef std::map<std::string, Node> Map;

    typedef Sequence::iterator iterator;
    typedef Sequence::const_iterator const_iterator;
    typedef Map::iterator map_iterator;
    typedef Map::const_iterator map_const_iterator;
  private:
    std::string key;
    std::string value;
    Sequence sequence;
    Map map;
    Types::Type type;
    int indent;
    Node* parent;
    friend class Parser;
  public:
    Node();
    Node(const std::string& key);
    template <typename T>
    Node(const std::string& key, const T& value)
      : key(key), sequence(), map(), type(Types::Scalar), indent(0) {
      std::stringstream ss;
      ss << value;
      if (ss.fail())
        this->type = Types::Undefined;
      else
        this->value = ss.str();
    }
    template <>
    Node(const std::string& key, const Sequence& sequence)
      : key(key), sequence(sequence),
      type(Types::Sequence), indent(0) {}
    template <>
    Node(const std::string& key, const Map& map)
      : key(key), map(map),
      type(Types::Map), indent(0) {}
    Node(const Node& other);
    ~Node() {}
    inline const std::string& getKey() const {
      return this->key;
    }
    inline const std::string& getValue() const {
      return this->value;
    }
    inline const Sequence& getSequence() const {
      return this->sequence;
    }
    inline Sequence& getSequence() {
      return this->sequence;
    }
    inline const Map& getMap() const {
      return this->map;
    }
    inline Map& getMap() {
      return this->map;
    }
    inline Types::Type getType() const {
      return this->type;
    }
    inline Node* getParent() const {
      return this->parent;
    }

    const Node& operator[](const std::string& key) const;
    const Node& operator[](const size_t index) const;

    Node& operator[](const std::string& key);
    Node& operator[](const size_t index);

    std::string toString() const;
    operator std::string() const;

    template <typename T>
    T as() const {
      T value;
      std::stringstream ss(this->value);
      ss >> value;
      if (ss.fail())
        throw std::runtime_error("Failed to convert " + this->value + " to " + typeid(T).name());
      return value;
    }

    template <>
    std::string as<std::string>() const {
      return this->value;
    }

    template <>
    bool as<bool>() const {
      if (this->value == "true")
        return true;
      else if (this->value == "false")
        return false;
      else if (this->as<int>() != 0)
        return true;
      else if (this->as<double>() != 0.0)
        return true;
      else if (this->as<int>() == 0)
        return false;
      else if (this->as<double>() == 0.0)
        return false;
      else
        throw std::runtime_error("Expected a boolean, got: " + this->value);
    }

    template <Types::Type T>
    inline bool is() const { return this->type == T; }
    inline bool isValid() const { return this->type != Types::Undefined; };

    inline void setKey(const std::string& key) { this->key = key; };

    template <typename T>
    Node& operator=(const T& val) {
      if (!this->is<Types::Scalar>())
        throw std::runtime_error("Node is not a scalar");
      std::stringstream ss;
      ss << val;
      this->value = ss.str();
      return *this;
    }

    template <>
    Node& operator=(const Node& other) {
      if (this == &other) return *this;
      this->key = other.key;
      this->value = other.value;
      this->sequence = other.sequence;
      this->map = other.map;
      this->type = other.type;
      this->indent = other.indent;
      return *this;
    }

    template <>
    Node& operator=(const Map& other) {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Node is not a map");
      this->map = other;
      return *this;
    }

    template <>
    Node& operator=(const Sequence& other) {
      if (!this->is<Types::Sequence>())
        throw std::runtime_error("Node is not a sequence");
      this->sequence = other;
      return *this;
    }

    template <typename T>
    inline typename T::iterator begin();
    template <typename T>
    inline typename T::const_iterator begin() const;
    template <typename T>
    inline typename T::iterator end();
    template <typename T>
    inline typename T::const_iterator end() const;

    template <>
    inline Map::const_iterator begin<Map>() const {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
      return this->map.begin();
    }
    template <>
    inline Sequence::const_iterator begin<Sequence>() const {
      if (!this->is<Types::Sequence>())
        throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
      return this->sequence.begin();
    }
    template <>
    inline Map::iterator begin<Map>() {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
      return this->map.begin();
    }
    template <>
    inline Sequence::iterator begin<Sequence>() {
      if (!this->is<Types::Sequence>())
        throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
      return this->sequence.begin();
    }

    template <>
    inline Map::const_iterator end<Map>() const {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
      return this->map.end();
    }
    template <>
    inline Sequence::const_iterator end<Sequence>() const {
      if (!this->is<Types::Sequence>())
        throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
      return this->sequence.end();
    }
    template <>
    inline Map::iterator end<Map>() {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
      return this->map.end();
    }
    template <>
    inline Sequence::iterator end<Sequence>() {
      if (!this->is<Types::Sequence>())
        throw std::runtime_error("Expected a Sequence, got: " + Types::GetLabel(this->type));
      return this->sequence.end();
    }

    template <typename T>
    inline Node& last();
    template <typename T>
    inline const Node& last() const;

    template <>
    inline const Node& last<Map>() const {
      return this->map.rbegin()->second;
    }

    template <>
    inline const Node& last<Sequence>() const {
      return *this->sequence.rbegin();
    }

    template <>
    inline Node& last<Map>() {
      return this->map.rbegin()->second;
    }

    template <>
    inline Node& last<Sequence>() {
      return *this->sequence.rbegin();
    }

    inline bool has(const std::string& key) const {
      if (!this->is<Types::Map>())
        throw std::runtime_error("Expected a Map, got: " + Types::GetLabel(this->type));
      return this->map.count(key) > 0;
    }

    const Node& insert(const Node& node);
    void remove(const std::string& key);
    void remove(size_t idx);
    template <typename T>
    void remove(typename T::iterator it);
    template <typename T>
    void remove(typename T::const_iterator it);

    template <>
    void remove<Map>(Map::iterator it) {
      this->map.erase(it);
    }

    template <>
    void remove<Sequence>(Sequence::iterator it) {
      this->sequence.erase(it);
    }

    uint32_t size() const;
    static Node NewNull(const std::string& key);
    template <typename T>
    static Node NewScalar(const std::string& key, const T& value) {
      return Node(key, value);
    }
    static Node NewMap(const std::string& key);
    static Node NewSequence(const std::string& key);
  };

  class Parser {
  private:
    std::stringstream doc;
    std::string scalar;
    Node root;
    Node* current;
    std::stack<Node*> stack;
  public:
    Parser(const std::stringstream& stream);
    Parser(const std::string& path);
    ~Parser() {}
    inline Node& getRoot() { return this->root; }
    inline const Node& getRoot() const { return this->root; }
    void parse(bool skipIndent = false, bool skipTokens = false);

  private:
    // skips doc's whitespace and counts its indentation
    void skipWhitespace();
    int countWhitespace();
    bool isEmptyLine();
    void skipComment();

    void handleContext(int indent, const Node* node);

    struct ScalarContext {
      enum Type {
        None = -1,
        Normal,
        SingleQuoted,
        DoubleQuoted
      };
    };
    std::string retrieveScalar(bool ignoreTokens = false);
    void parseScalarContext(ScalarContext::Type& ctx, char c);
    void removeScalarQuotes(std::string& scalar);
  };
  Node LoadFile(const std::string& path);

  void RunTests();
}
std::ostream& operator<<(std::ostream& os, const YAML::Node& node);