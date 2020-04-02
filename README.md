## IW3xRadiant - A Call of Duty 4 Radiant Modification
This project is aimed at developers and includes various modifications/additions and was built to be used with IW3xo.  
Running both, IW3xRadiant and IW3xo, enables a live-link between CoD4 and Radiant.

Build project:
1. Hit "generate.bat" to build project files
2. Load the project and open project-settings to setup paths (Debug/Release):
   - General:   output directory path -> "path-to-cod4-root\bin\"
   - Debugging: command -> "path-to-cod4-root\bin\IW3xRadiant.exe"
   - Debugging: working directory -> "path-to-cod4-root\bin\"
3. Build the solution
4. Copy everything within the assets folder into your cod4-root

Requires:
- https://github.com/xoxor4d/iw3xo-dev
