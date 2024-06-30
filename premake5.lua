
OUTPUT_DIR = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
ROOT_DIR = path.getabsolute(".")
PROJECT_TARGET_DIR = path.join(ROOT_DIR, "bin", OUTPUT_DIR, "%{prj.name}")
PROJECT_VENDOR_DIR = path.join(ROOT_DIR, "bin", OUTPUT_DIR, "vendor", "%{prj.name}")
PROJECT_OBJ_DIR = path.join(ROOT_DIR, "objs", OUTPUT_DIR, "%{prj.name}")

function vendorFiles(lib)
  local vendorPath = path.join(ROOT_DIR, "vendor", lib)
  return function(...)
    return path.join(vendorPath, ...);
  end
end

function vendor(lib)
  include (path.join("vendor","scripts", lib .. ".lua"))
end

workspace "Webserv"
  architecture "x64"
  configurations { "Debug", "Release" }
  flags { "MultiProcessorCompile" }

  filter {"system:linux", "configurations:Debug"}
    buildoptions { "-gdwarf-2" }
  filter {}

group "Vendor"
  vendor "lua"
group ""

group "Core"
  include "project"
group ""