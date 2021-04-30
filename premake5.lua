workspace "at_odds"
   configurations { "Debug", "Release" }

project "at_odds"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   
   pchheader "gamepch.h"
   pchsource "src/gamepch.cpp"

   files { "**.h", "**.cpp" }
   
   filter "configurations:Debug"
      kind "ConsoleApp"
      defines { "_DEBUG" }
      architecture "x86_64"
      symbols "On"

   filter "configurations:Release"
      kind "WindowedApp"
      entrypoint "mainCRTStartup"
      defines { "NDEBUG" }
      architecture "x86_64"
      optimize "On"