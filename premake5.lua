workspace "at_odds"
   configurations { "Debug", "Release" }

project "at_odds"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   
   pchheader "gamepch.h"
   pchsource "src/gamepch.cpp"

   files { "**.h", "**.cpp" }
   
   filter "configurations:Debug"
      defines { "_DEBUG" }
      architecture "x86_64"
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      architecture "x86_64"
      optimize "On"