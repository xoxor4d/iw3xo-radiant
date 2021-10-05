## IW3xRadiant - A Call of Duty 4 Radiant Modification using ImGui
This project is aimed at developers and includes various modifications/additions and was built to be used with IW3xo.  
Running both, IW3xRadiant and IW3xo, enables a live-link between CoD4 and Radiant. You can, however, just use it as a direct replacement for stock radiant.

<br>

Current goals:
- replace most if not all of the MFC/Windows UI with Dear ImGui
- extend functionalities across the board

<br>

![image](https://user-images.githubusercontent.com/45299104/136065983-e049e9c6-eb5f-43bf-83a0-7c51f6394a16.jpg)
![image](https://user-images.githubusercontent.com/45299104/136064749-f4dcde48-5e08-4d1e-8c9d-5ffc3e9e54cc.jpg)

<center>^ build the layout you've always wanted</center>

___

<br>

### Pre-build binaries
https://github.com/xoxor4d/iw3xo-radiant/releases

<br>

### How to build/compile the project yourself?
1. Hit "generate.bat" to build project files with premake
2. Load the project in Visual Studio and open project-settings to setup paths (Debug/Release):
   - General:   output directory path -> "path-to-cod4-root\bin\"
   - Debugging: command -> "path-to-cod4-root\bin\IW3xRadiant.exe"
   - Debugging: working directory -> "path-to-cod4-root\bin\"
3. Build -> Build Solution
4. Copy everything within the assets folder into your cod4-root

<br>

___

<br>

Optional:  
https://github.com/xoxor4d/iw3xo-dev

Project Page:  
https://xoxor4d.github.io/projects/iw3xo-radiant/

Discord:  
https://discord.gg/t5jRGbj

<br>

## Credits
- The IW4x Team
- The Plutonium Project Team
- Nukem
- https://github.com/Nukem9/LinkerMod
- https://github.com/Nukem9/detours
- https://github.com/ocornut/imgui
- https://github.com/devKlausS/dxsdk

<br>

## Images

![image](https://user-images.githubusercontent.com/45299104/136064807-e08d54bd-e11a-48ae-a01d-1fe87fc4159c.jpg)
![image](https://user-images.githubusercontent.com/45299104/131264943-5bbb6cca-1853-4c77-b520-3ba76384abc5.png)

<br>

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
