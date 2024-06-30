project "Webserv"
  kind "StaticLib"
  language "C++"
  cppdialect "C++20"
  staticruntime "on"

  targetdir (PROJECT_TARGET_DIR)
  objdir (PROJECT_OBJ_DIR)

  files {
    "includes/**.h",
    "includes/**.hpp",
    "src/**.cpp"
  }

  includedirs {
    "includes",
    vendorFiles("lua")(),
  }

  links {
    "lua"
  }

  filter "system:windows"
    defines { '_WIN32' }

  filter "system:linux"
    defines { '_LINUX' }

  filter "system:windows"
    systemversion "latest"

  filter "system:linux"
    pic "On"
    systemversion "latest"

  filter "configurations:Debug"
    defines "_DEBUG"
    runtime "Debug"
    symbols "on"

  filter "configurations:Release"
    defines "_RELEASE"
    runtime "Release"
    optimize "on"
