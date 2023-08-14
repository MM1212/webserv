#pragma once

#include <utils/Instance.hpp>
#include <fstream>
#include <utils/misc.hpp>
#include <utils/Logger.hpp>
#include <Settings.hpp>
#include <string>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>



namespace HTTP {
  namespace Routing {
    class Static;
  }
  class DirectoryBuilder {
  public:
    struct Flags {
      enum Flag {
        File = 1,
        Directory = 2,
        Hidden = 4,
        Executable = 8,
        Symlink = 16,
      };
    };
    class Entry {
      std::string name;
      std::string folder;
      std::string url;
      uint32_t flags;
      uint32_t size;
      uint32_t lastModified;
    public:
      Entry(const std::string& name, const std::string& folder, const std::string& url, uint32_t flags, uint32_t size, uint32_t lastModified);
      Entry(const std::string& path, const std::string& url);
      ~Entry();
      Entry(const Entry& other);
      Entry& operator=(const Entry& other);
      inline const std::string& getName() const { return this->name; }
      inline const std::string& getFolder() const { return this->folder; }
      inline const std::string& getUrl() const { return this->url; }
      inline uint32_t getFlags() const { return this->flags; }
      inline uint32_t getSize() const { return this->size; }
      inline uint32_t getLastModified() const { return this->lastModified; }

      inline std::string getPath() const { return Utils::resolvePath(2, this->folder.c_str(), this->name.c_str()); }
      template <Flags::Flag T>
      inline bool is() const { return this->flags & T; }
      void load();
      operator std::string() const;
      std::string buildFlags() const;
      std::string getIcon(const std::string& icon) const;
    };
  public:
    std::string build(const std::string& path, const Routing::Static& route) const;
  private:
    DirectoryBuilder();
    void loadTemplate(const std::string& path);
    std::string templateFile;
    friend class ::Instance;
  };
}