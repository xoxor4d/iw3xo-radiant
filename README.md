


<h1 align="center">IW3xRadiant - A Call of Duty 4 Radiant Modification using ImGui</h3>

<p align="center">
This project is aimed at developers and includes various modifications/additions and was initially built to be used with IW3xo.
Running IW3xRadiant and IW3xo enables a live-link between CoD4 and Radiant. You can, however, just use it as a direct replacement for stock radiant.
</p>

<br>
<div align="center" markdown="1">

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/xoxor4d/iw3xo-radiant?color=%2368BC71&logo=github)](https://github.com/xoxor4d/iw3xo-radiant/releases)&ensp;
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/xoxor4d/iw3xo-radiant/latest/develop?logo=github)&ensp;
[![Downloads](https://img.shields.io/github/downloads/xoxor4d/iw3xo-radiant/total?logo=github&label=total-downloads)](https://github.com/xoxor4d/iw3xo-radiant/releases)&ensp;
[![Discord](https://img.shields.io/discord/677574256678141973?label=Discord&logo=discord&logoColor=%23FFFF&)](https://discord.gg/t5jRGbj)&ensp;

<br>

### nightly builds - develop branch
( download and install the [latest release](https://github.com/xoxor4d/iw3xo-radiant/releases) before using nightly's )

[![build-develop](https://img.shields.io/github/workflow/status/xoxor4d/iw3xo-radiant/Build/develop?logo=github&label=nightly-develop)](https://nightly.link/xoxor4d/iw3xo-radiant/workflows/build/develop/Debug%20binaries.zip)&ensp;
[![build-release](https://img.shields.io/github/workflow/status/xoxor4d/iw3xo-radiant/Build/develop?logo=github&label=nightly-release)](https://nightly.link/xoxor4d/iw3xo-radiant/workflows/build/develop/Release%20binaries.zip)&ensp;

<br>

### Project Page (Guide / In-Depth)
https://xoxor4d.github.io/projects/iw3xo-radiant

</div>

<br>

![image](https://user-images.githubusercontent.com/45299104/143088571-8a392c17-ed21-4639-9afb-f4fa492e3fca.jpg)

![](readme/effects.gif)

![image](readme/feat_ui.gif)
<div align="center" markdown="1">

^ build the layout you've always wanted  
[https://www.youtube.com/watch?v=YSIGsSZZkco](https://www.youtube.com/watch?v=YSIGsSZZkco)
</div>

___
## Features

<br>

### Goals:
- replace most if not all of the MFC/Windows UI with Dear ImGui
- extend functionalities across the board

### New features not found within the original cod4 radiant:
- play, edit and export effects as createFx files right from within radiant (makes effectsEd _almost_ obsolete)
- switch / scale / place the individual windows however you want
- preview xmodels and drag them directly into the scene using the model previewer
- custom lighting shader with normal-mapping, specular highlights, reflections and fog
- ability to limit shadow drawing distance when using stock sunpreview (++FPS)
- filmtweak support
- render actual water instead of case-textures 
- guizmo to manipulate entities and brushes from within the camera window
- live link (sync. brushes (with collision), camera and worldspawn settings between cod4 and radiant)
- high poly xmodel's no longer crash radiant
- realtime viewports
- better surface / property editor
- context aware grid and camera context menus with QoL features
- better vertex edit dialog
- zoom to cursor
- editable toolbars, hotkeys, colors (all saved)
- new file dialogs with working default paths
- texture window toolbar for quick filtering
- rope/wire generator
- sun direction visualizer
- a proper console with dvar support (incl. dvar suggestions and autocomplete)
- print parsed entity and brush num on map load making it easier to find issues in map files (off by default) 
- increased undo limit
- alot of QOL features

<br>

___
## Build / Compile

<br>

### How to build / compile the project using Visual Studio
1. Clone the repo! __(zip does not include deps!)__
2. Copy everything within the assets folder into your cod4-root folder
3. Use __update_submodules.bat__ to update submodules
4. Use __generate-buildfiles.bat__ to build project files with premake
5. Load the solution `(build/iw3xo-radiant.sln)` and open the iw3r project-settings to setup paths:
> - General &ensp; &ensp; &ensp; output directory path -> `path-to-cod4-root\bin\`
> - Debugging &ensp; command -> `path-to-cod4-root\bin\IW3xRadiant.exe`
> - Debugging &ensp; working directory -> `path-to-cod4-root\bin\`
6. Build -> Build Solution or run with debugger

<br>

### How to build the project using VSCode
1. Install C++ Build tools (msbuild)
2. Add msbuild folder to the "PATH" environment variable:
> `%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin`
3. Add `COD4_ROOT` environment variable with path to your cod4 directory 
> `"COD4_ROOT" "D:\COD4-Modtools"`
4. Clone the repo (zip does not include deps!)
5. Copy everything within the assets folder into your cod4-root folder
6. Drag and drop all files from `.vscode/tracked\` into `.vscode\`
7. Open __iw3xo-radiant.code-workspace__ 
8. Run task: `update_submodules` or open __update_submodules.bat__
9. Run task: `generate-buildfiles` or open __generate-buildfiles.bat__

Use provided build-tasks with the option to copy `iw3r.dll` and `iw3r.pdb` to `%COD4_ROOT%/bin`  
__Run->Start Debugging__ will build and copy a debug build to to `%COD4_ROOT%/bin` and launch IW3xRadiant.

<br>

___

<br>

### Optional:  
https://github.com/xoxor4d/iw3xo-dev

### Project Page:  
https://xoxor4d.github.io/projects/iw3xo-radiant/

### Discord:  
https://discord.gg/t5jRGbj

<br>

## Credits
- [X Labs](https://github.com/XLabsProject) (especially [Snake](https://github.com/momo5502))
- [The Plutonium Project Team](https://plutonium.pw/) (especially [Rektinator](https://github.com/RektInator))
- [Nukem9 - LinkerMod](https://github.com/Nukem9/LinkerMod) & [detours](https://github.com/Nukem9/detours)
- [ocornut - Dear ImGui](https://github.com/ocornut/imgui)
- [CedricGuillemet - ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [nlohmann - fifo_map](https://github.com/nlohmann/fifo_map)
- [David Gallardo - imgui_color_gradient](https://gist.github.com/galloscript/8a5d179e432e062550972afcd1ecf112)
- [nem0 - ImGui CurveEditor](https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930)
- [Infinity Ward - OG. Radiant and Effects Framework](https://www.infinityward.com)
- [id-Software - OG. Radiant](https://github.com/id-Software/Quake-III-Arena/tree/master/q3radiant)

<br>

___
## Images

<br>

### Model Selector / Preview:
![image](https://user-images.githubusercontent.com/45299104/146609277-803b9203-4a64-4355-99fc-32cfb7ad6d82.jpg)

### Custom Lighting Shader On/Off:
![image](https://user-images.githubusercontent.com/45299104/146609441-50255d3c-0dc0-4f9a-894e-373abf4eafc6.jpg)
![image](https://user-images.githubusercontent.com/45299104/146609445-e11ae289-641e-4979-915b-bbb0c6b4942b.jpg)

### Effect Playback
![image](https://xoxor4d.github.io/assets/img/iw3xo-radiant/gif/radiant_effect_leaves.gif)

<br>

### Effects Editor
<p float="left">
  <img src="https://user-images.githubusercontent.com/45299104/159173092-10d91c68-094d-464d-ade4-ea90cdcd387a.jpg" width="47%" />
  <img src="https://user-images.githubusercontent.com/45299104/159173144-1fd8e23e-2d39-4621-918e-884f37e83968.jpg" width="47%" align="right" /> 
</p>

![image](https://user-images.githubusercontent.com/45299104/159173151-f54c7bb6-6b1f-438b-b667-f04747147a97.jpg)


<br>

## Disclaimer
This software has been created purely for the purposes of academic research. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
