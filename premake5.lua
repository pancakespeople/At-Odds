workspace "at_odds"
   configurations { "Debug", "Release" }

project "at_odds"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   
   pchheader "src/gamepch.h"
   pchsource "src/gamepch.cpp"

   files { "**.h", "**.cpp" }
   
   filter "system:Windows"
      pchheader "gamepch.h"
   
   filter "system:Linux"
      links {
         "sfml-graphics",
         "sfml-window",
         "sfml-system",
         "sfml-audio",
         "boost_serialization",
         "tgui"
      }
   
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
