-- https://github.com/citizenfx/fivem/blob/master/code/vendor/lua54.lua
project "lua"
  language "C++"
  kind "StaticLib"

  if os.istarget('windows') then
    flags { "LinkTimeOptimization" }

    -- longjmp *should* be exception-safe on Windows non-x86
    defines { "LUA_USE_LONGJMP" }
  elseif os.istarget('linux') then
    defines { "LUA_USE_POSIX" }
  end

  defines {
    'MAKE_LIB', -- Required specification when building onelua.c
    'NOMINMAX',

    --[[ Lua Flags ]]
    'LUA_COMPAT_5_3',
    -- 'LUA_NO_BYTECODE', -- Disables the usage of lua_load with binary chunks

    --[[ Lua Extensions ]]
    -- 'LUA_SANDBOX', -- Disable many features within ldblib.c
    'LUA_C99_MATHLIB', -- Include c99 math functions in lmathlib
    -- disabled (worse yield performance)
    --'LUA_CPP_EXCEPTIONS', -- @EXPERIMENT: unprotected calls are wrapped in typed C++ exceptions
    'GRIT_POWER_COMPOUND', -- Add compound operators
    'GRIT_POWER_INTABLE', -- Support for unpacking named values from tables using the 'in' keyword
    'GRIT_POWER_TABINIT', -- Syntactic sugar to improve the syntax for specifying sets
    'GRIT_POWER_SAFENAV', -- An indexing operation that suppresses errors on accesses into undefined table
    'GRIT_POWER_CCOMMENT', -- Support for C-style block comments
    'GRIT_POWER_DEFER_OLD', -- Import func2close from ltests.h into the base library as _G.defer
    'GRIT_POWER_JOAAT', -- Enable compile time Jenkins' one-at-a-time hashing
    'GRIT_POWER_EACH', -- __iter metamethod support; see documentation
    'GRIT_POWER_WOW', -- Expose lua_createtable and other compatibility functions common in other custom runtimes
    'GRIT_POWER_CHRONO', -- Enable nanosecond resolution timers and x86 rdtsc sampling in loslib.c
    'GRIT_COMPAT_IPAIRS', -- Reintroduce compatibility for the __ipairs metamethod
    'GRIT_POWER_BLOB', -- Enable an API to create non-internalized contiguous byte sequences

  }

  removedefines {
    'LUA_INCLUDE_LIBGLM',
  }

  files {
    "../lua/onelua.c",
  }

  filter { "files:**.c" }
    compileas "C++"

  filter { "configurations:Release" }
    optimize "Speed"
    runtime "Release"

  filter { "configurations:Debug" }
    symbols "On"
    runtime "Debug"