#pragma once

#include <string>
#include <map>

namespace HTTP {
  class Headers {
  public:
    Headers();
    ~Headers();
    Headers(const Headers& other);
    Headers& operator=(const Headers& other);
    void clear();
    bool append(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    void remove(const std::string& key);
    bool has(const std::string& key) const;
    const std::string& get(const std::string& key) const;

    const std::map<std::string, std::string>& getAll() const;

    operator std::string() const;
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Headers& headers);
  private:
    std::map<std::string, std::string> headers;
  };

}