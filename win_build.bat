echo Getting dependencies...
git clone https://github.com/microsoft/vcpkg
vcpkg\bootstrap-vcpkg.bat
vcpkg\vcpkg install sfml tgui boost-serialization --triplet=x64-windows
vcpkg\vcpkg integrate install

echo Building...
premake5.exe vs2019
msbuild /m /p:Configuration=Release .