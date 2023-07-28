#include "HTTP.hpp"

using HTTP::Routes;

std::string Routes::GetLabel(const Type type) {
  switch (type) {
  case Routes::Normal:
    return "Normal";
  case Routes::File:
    return "File";
  case Routes::Directory:
    return "Directory";
  case Routes::Upload:
    return "Upload";
  }
  return "Unknown";
}