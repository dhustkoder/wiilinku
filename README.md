# WiiLinkU
Open Source Tool for the WiiU and Windows Desktop

Devs Discord: [https://discord.gg/Nq57FcG](https://discord.gg/Nq57FcG)

Connects the WiiU Gamepad and Wiimotes to Windows PC as XBOX Controller

Planned Features:
- Game Streaming to WiiU TV/Gamepad (if possible)
- Save configurations
- Graphical UI on WiiU / Better GUI on Desktops
- Button Mapping
- Linux Support

# ViGEmBus
You'll need ViGEmBus driver installed to use wiilinku.exe
[https://github.com/ViGEm/ViGEmBus](https://github.com/ViGEm/ViGEmBus)

# Cloning
git clone --recursive https://github.com/dhustkoder/wiilinku

# Compiling
### WiiU
- make -f wiiu.mak BUILD_TYPE=Release

### Windows
- compile externals/ViGEmClient first using VS2019
- windows.bat release

