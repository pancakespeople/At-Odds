# At-Odds
Space 4x/rts game

![Discord](https://img.shields.io/discord/891524090131775548?style=plastic)
![GitHub all releases](https://img.shields.io/github/downloads/pancakespeople/At-Odds/total)
![YouTube Channel Views](https://img.shields.io/youtube/channel/views/UCIUXVi3ZoedXWo1I7xZqjoA?style=social)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/pancakespeople/At-Odds?style=plastic)

![1](img/1.png)

![2](img/2.png)

![3](img/3.png)


# Building

Linux (Debian-based distros):

Run linux_build.sh

Windows:

Clone the vcpkg repo

	git clone https://github.com/microsoft/vcpkg

In the vcpkg folder, run bootstrap-vcpkg.bat

Run win_build.ps1 and then open the generated Visual Studio solution.

Dependencies:

	sfml:x64 2.5.1
	tgui:x64 0.9.1
	boost-serialization:x64 1.75.0
