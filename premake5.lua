workspace "at_odds"
   configurations { "Debug", "Release" }

project "at_odds"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs "src/pch"
   
   pchheader "gamepch.h"
   pchsource "src/gamepch.cpp"

   files { "src/**.h", "src/**.hpp", "src/**.cpp" }
   
   filter "system:Windows"
      buildoptions "/bigobj"
   
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
