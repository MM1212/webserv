#include "http/DirectoryBuilder.hpp"
#include "http/routing/modules/Static.hpp"

using HTTP::DirectoryBuilder;

DirectoryBuilder::Entry::Entry(const std::string& name, const std::string& folder, const std::string& url, uint32_t flags, uint32_t size, uint32_t lastModified)
  : name(name), folder(folder), url(url), flags(flags), size(size), lastModified(lastModified) {}

DirectoryBuilder::Entry::Entry(const std::string& path, const std::string& url)
  : name(""), folder(""), url(url), flags(0), size(0), lastModified(0) {
  this->name = Utils::basename(path);
  this->folder = Utils::dirname(path);
  this->load();
}

DirectoryBuilder::Entry::~Entry() {}

DirectoryBuilder::Entry::Entry(const Entry& other)
  : name(other.name), folder(other.folder), flags(other.flags), size(other.size), lastModified(other.lastModified) {}

DirectoryBuilder::Entry& DirectoryBuilder::Entry::operator=(const Entry& other) {
  if (this == &other) return *this;
  this->name = other.name;
  this->folder = other.folder;
  this->flags = other.flags;
  this->size = other.size;
  this->lastModified = other.lastModified;
  return *this;
}

void DirectoryBuilder::Entry::load() {
  struct stat st;
  std::string path = this->getPath();
  if (lstat(path.c_str(), &st) == -1)
    throw std::runtime_error("Failed to stat file: " + path);
  this->flags = 0;
  if (S_ISDIR(st.st_mode))
    this->flags |= Flags::Directory;
  else if (S_ISREG(st.st_mode)) {
    this->flags |= Flags::File;
    if (st.st_mode & S_IXUSR)
      this->flags |= Flags::Executable;
  }
  else if (S_ISLNK(st.st_mode))
    this->flags |= Flags::Symlink;
  else
    throw std::runtime_error("Unsupported file type: " + path);
  if (this->name[0] == '.')
    this->flags |= Flags::Hidden;
  this->size = st.st_size;
  if (this->is<Flags::Directory>()) {
    this->size = 0;
    if (*this->url.rbegin() != '/')
      this->url += '/';
    if (*this->name.rbegin() != '/')
      this->name += '/';
  }
  this->lastModified = st.st_mtime;
}

std::string DirectoryBuilder::Entry::getIcon(const std::string& icon) const {
  std::stringstream ss;
  ss << "<span class=\"iconify\" data-icon=\"mdi-"
    << icon
    << "\"></span>";
  return ss.str();
}

std::string DirectoryBuilder::Entry::buildFlags() const {
  std::stringstream result;
  std::string hiddenFlag(this->is<Flags::Hidden>() ? "-hidden" : "");
  result << "<div class=\"dir-flags-wrapper\" data-flags=\"" << this->getFlags() << "\">"
    << (this->is<Flags::File>() && !this->is<Flags::Executable>() ? this->getIcon("file" + hiddenFlag) : "")
    << (this->is<Flags::Directory>() ? this->getIcon("folder" + hiddenFlag) : "")
    << (this->is<Flags::Symlink>() ? this->getIcon("link") : "")
    << (this->is<Flags::Executable>() ? this->getIcon("console") : "")
    << "</div>";
  return result.str();
}

DirectoryBuilder::Entry::operator std::string() const {
  std::stringstream ss;
  ss << "<tr class=\"dir-entry\">"
    << "<td class=\"dir-name\" data-value=\""
    << this->name
    << "\">"
    << "<a href=\"" << this->url << "\">"
    << this->name
    << "</a>"
    << "</td>"
    << "<td class=\"dir-last-modified\" data-value=\""
    << this->lastModified
    << "\">";
  if (this->lastModified != 0)
    ss << this->lastModified << "</td>";
  else
    ss << "-" << "</td>";
  ss << "<td class=\"dir-size\" data-value=\""
    << this->size
    << "\">";
  if (this->size != 0)
    ss << this->size << "</td>";
  else
    ss << "-" << "</td>";
  ss
    << "<td class=\"dir-flags\" data-value=\""
    << this->getFlags()
    << "\">" << this->buildFlags() << "</td>"
    << "</tr>";
  return ss.str();
}

DirectoryBuilder::DirectoryBuilder() {
  this->loadTemplate(Instance::Get<Settings>()->get<std::string>("http.static.directory_builder_template"));
}

void DirectoryBuilder::loadTemplate(const std::string& path) {
  std::ifstream file(path.c_str());
  if (!file.is_open())
    throw std::runtime_error("Failed to open directory builder template: " + path);
  std::stringstream ss;
  ss << file.rdbuf();
  std::string line;
  while (std::getline(ss, line))
    this->templateFile += Utils::trim(line);
  file.close();
}



std::string DirectoryBuilder::build(const std::string& path, const Routing::Static& route) const {
  std::string file(this->templateFile);
  DIR* dir = opendir(path.c_str());
  if (!dir)
    throw std::runtime_error("Failed to open directory: " + path);
  struct dirent* ent;
  std::string listing;
  listing += Entry("..", "", "..", Flags::Directory, 0, 0);
  while ((ent = readdir(dir))) {
    std::string fileName(ent->d_name);
    if (
      fileName == "." ||
      fileName == ".." ||
      (fileName.find(".") == 0 && route.ignoreHiddenFiles()))
      continue;
    const std::string filePath(Utils::resolvePath(2, path.c_str(), ent->d_name));
    const std::string urlPath(Utils::resolvePath(2, ".", ent->d_name));
    Entry entry(filePath, urlPath);
    listing += entry;
  }
  std::string rootPath(path);
  rootPath.erase(0, route.getRoot().size() - 1);
  file.replace(file.find("{{path}}"), 8, rootPath);
  file.replace(file.find("{{path}}"), 8, rootPath);
  file.replace(file.find("{{files}}"), 9, listing);
  closedir(dir);
  return file;
}