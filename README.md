## IW3xRadiant - A Call of Duty 4 Radiant Modification using ImGui

<p align="center">
	This project is aimed at developers and includes various modifications/additions and was built to be used with IW3xo.  
Running both, IW3xRadiant and IW3xo, enables a live-link between CoD4 and Radiant. You can, however, just use it as a direct replacement for stock radiant.

</p>

<br>

### Goals:
- replace most if not all of the MFC/Windows UI with Dear ImGui
- extend functionalities across the board

### New features not found within the original cod4 radiant:
- switch/scale/place the individual windows however you want
- preview xmodels and drag them directly into the scene using the model previewer/selector
- guizmo to manipulate entities and brushes from within the camera window
- live link
- realtime viewports
- better property editor 
- zoom to cursor
- editable toolbars, hotkeys, colors (all saved)
- a proper console with dvar support (incl. dvar suggestions and autocomplete)
- increased undo limit
- alot of QOL features

<br>

![image](https://user-images.githubusercontent.com/45299104/136064749-f4dcde48-5e08-4d1e-8c9d-5ffc3e9e54cc.jpg)
![image](https://user-images.githubusercontent.com/45299104/136065983-e049e9c6-eb5f-43bf-83a0-7c51f6394a16.jpg)
![image](https://user-images.githubusercontent.com/45299104/143088571-8a392c17-ed21-4639-9afb-f4fa492e3fca.jpg)


<p align="center">
	<b>^ build the layout you've always wanted</b><br>
    <b>Youtube preview: </b>
    <a href="https://www.youtube.com/watch?v=YSIGsSZZkco">https://www.youtube.com/watch?v=YSIGsSZZkco</a><br>
</p>

___

### Pre-build binaries
https://github.com/xoxor4d/iw3xo-radiant/releases

<br>

### How to build (Visual Studio)
1. Clone the repo (zip does not include deps!)
2. Copy everything within the assets folder into your cod4-root folder
3. Use "update_submodules.bat" to update submodules
4. Use "generate-buildfiles.bat" to build project files with premake
5. Load the solution (build/iw3xo-radiant.sln) in visual studio and open the iw3r project-settings to setup paths (Debug/Release):
   - General:   output directory path -> "path-to-cod4-root\bin\"
   - Debugging: command -> "path-to-cod4-root\bin\IW3xRadiant.exe"
   - Debugging: working directory -> "path-to-cod4-root\bin\"
6. Build -> Build Solution

<br>

### How to build (VSCode)
1. Install C++ Build tools (msbuild)
2. Add msbuild folder to the "PATH" environment variable (eg. "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin")
3. Add "COD4_ROOT" environment variable with path to your cod4 directory (eg. "COD4_ROOT" "D:\COD4-Modtools")
4. Clone the repo (zip does not include deps!)
5. Copy everything within the assets folder into your cod4-root folder
6. Open "iw3xo-radiant.code-workspace" 
7. Run task "update_submodules" or open "update_submodules.bat"
8. Run task "generate-buildfiles" or open "generate-buildfiles.bat"

Use provided build-tasks to build debug/release builds with the option to copy iw3r.dll and iw3r.pdb to %COD4_ROOT%/bin.
Run->Start Debugging will build and copy a debug build to to %COD4_ROOT%/bin and launch IW3xRadiant.

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
- The IW4x Team (especially Snake)
- The Plutonium Project Team (especially Rektinator)
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
