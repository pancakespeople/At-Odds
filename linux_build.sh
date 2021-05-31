#!/bin/bash

echo "Getting dependencies"
sudo apt-get install libsfml-dev libboost-serialization-dev
sudo add-apt-repository ppa:texus/tgui
sudo apt-get update

echo "Building"
./premake5 gmake2
make config=release
