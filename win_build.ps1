echo "Getting dependencies..."
./vcpkg/vcpkg install sfml boost-serialization --triplet=x64-windows
./vcpkg/vcpkg install tgui[sfml]:x64-windows --head
./vcpkg/vcpkg integrate install

echo "Building..."
./premake5.exe vs2022
msbuild /m /p:Configuration=Release .