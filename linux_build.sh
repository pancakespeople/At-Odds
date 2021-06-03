#!/bin/bash

echo "Getting dependencies"
sudo add-apt-repository ppa:texus/tgui
sudo apt-get update
sudo apt-get install libsfml-dev libboost-serialization-dev libtgui-0.9-dev

echo "Building"
./premake5 gmake2
make config=release
