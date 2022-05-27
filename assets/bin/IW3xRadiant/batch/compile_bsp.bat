@ECHO OFF

:: ###########################################
:: 		SET UP VARIABLES
:: ###########################################

set bsppath=%~1
set mapsourcepath=%~2
set treepath=%~3
set mapname=%4
set parmBSPOptions=%~5
set parmLightOptions=%~6
set compileBSP=%7
set compileLight=%8

if "%parmBSPoptions%" == "-" (
	set parmBSPoptions=
)

if "%parmLightOptions%" == "-" (
	set parmLightOptions=
)

IF NOT EXIST "%bsppath%" mkdir "%bsppath%"

echo [1] [bsppath] %bsppath%
echo [2] [mapsourcepath] %mapsourcepath%
echo [3] [treepath] %treepath%
echo [4] [mapname] %mapname%
echo [5] [parmBSPOptions] %parmBSPOptions%
echo [6] [parmLightOptions] %parmLightOptions%
echo [7] [compileBSP] %compileBSP%
echo [8] [compileLight] %compileLight%

if "%compileBSP%" == "1" (

	echo [CSTATUS] Compiling BSP
	echo.
	
	copy "%mapsourcepath%%mapname%.map" "%bsppath%%mapname%.map"
	"%treepath%bin\cod4map" -platform pc -loadFrom "%mapsourcepath%%mapname%.map" %parmBSPOptions% "%bsppath%%mapname%"
)

if "%compileLight%" == "1" (
	
	echo [CSTATUS] Compiling Light
	echo.

	IF EXIST "%mapsourcepath%%mapname%.grid"	copy "%mapsourcepath%%mapname%.grid" "%bsppath%%mapname%.grid"
	"%treepath%bin\cod4rad" -platform pc %parmLightOptions% "%bsppath%%mapname%"
)

IF EXIST "%bsppath%%mapname%.map"	del "%bsppath%%mapname%.map"
IF EXIST "%bsppath%%mapname%.d3dprt"	del "%bsppath%%mapname%.d3dprt"
IF EXIST "%bsppath%%mapname%.d3dpoly"	del "%bsppath%%mapname%.d3dpoly"
IF EXIST "%bsppath%%mapname%.vclog"	del "%bsppath%%mapname%.vclog"
IF EXIST "%bsppath%%mapname%.grid"	del "%bsppath%%mapname%.grid"
IF EXIST "%bsppath%%mapname%.lin"	move "%bsppath%%mapname%.lin" "%mapsourcepath%%mapname%.lin"

exit /b 1