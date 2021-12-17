1.) Install C++ Build tools (msbuild)
2.) Add msbuild to the "PATH" environment variable (eg. "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin")
3.) Add "COD4_ROOT" environment variable with path to your cod4 directory (eg. "COD4_ROOT" "D:\COD4-Modtools")
4.) Drag and drop all files from ".vscode/tracked\" into ".vscode\ (we ignore all files in .vscode/ besides the readme)
5.) Open "iw3xo-radiant.code-workspace" 
6.) Run task "update_submodules" or open "update_submodules.bat"
7.) Run task "generate-buildfiles" or open "generate-buildfiles.bat"

Use provided build-tasks to build debug/release builds with the option to copy iw3r.dll and iw3r.pdb to %COD4_ROOT%/bin.
Run->Start Debugging will build and copy a debug build to to %COD4_ROOT%/bin and launch IW3xRadiant.