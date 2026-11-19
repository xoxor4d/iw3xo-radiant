# CoDOL Radiant: the standalone editor ( no CoD4 radiant, no hooks ).
#
#   cmake -S editor -B build-editor -G "Visual Studio 17 2022" -A x64
#   cmake --build build-editor --config Release
#
# -> build-editor/Release/CoDOLRadiant.exe  ( x64; -A Win32 works as well )
#
# The CoDOL parts of the radiant plugin ( src/codol, script workbench, game bridge ) are compiled from the same
# source files; editor/compat/std_include.hpp stands in for the plugin's precompiled header.

cmake_minimum_required(VERSION 3.20)
project(codol-radiant LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
# third party sources; override when the submodules are checked out somewhere else ( -DDEPS=<path> )
set(DEPS ${ROOT}/deps CACHE PATH "folder with imgui, ImGuizmo, rapidjson")

# ---- native libraries
set(CODOL_ZONE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# codol_linker ( --verify of a zone the editor built ) comes with the tools
set(CODOL_ZONE_BUILD_TOOLS ON CACHE BOOL "" FORCE)
add_subdirectory(${ROOT}/codol_zone ${CMAKE_BINARY_DIR}/codol_zone)
add_subdirectory(${ROOT}/codol_render ${CMAKE_BINARY_DIR}/codol_render)
add_subdirectory(${ROOT}/codol_view ${CMAKE_BINARY_DIR}/codol_view)
set(CODOL_EDIT_RAPIDJSON_INCLUDE ${DEPS}/rapidjson/include CACHE PATH "" FORCE)
add_subdirectory(${ROOT}/codol_edit ${CMAKE_BINARY_DIR}/codol_edit)
# the asset compiler: the Assets window links the library; codol_assetc.exe is built next to the editor
set(CODOL_ASSETS_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(CODOL_ASSETS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${ROOT}/codol_assets ${CMAKE_BINARY_DIR}/codol_assets)
set_target_properties(codol_assetc PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)

# ---- imgui ( docking )
file(GLOB IMGUI_SRC ${DEPS}/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SRC}
	${DEPS}/imgui/backends/imgui_impl_dx9.cpp
	${DEPS}/imgui/backends/imgui_impl_win32.cpp
	${DEPS}/imgui/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC ${DEPS}/imgui)
target_compile_options(imgui PRIVATE /W0 /MP /std:c++latest)
target_compile_definitions(imgui PRIVATE UNICODE _UNICODE $<$<CONFIG:Release>:NDEBUG>)

# ---- the editor
add_executable(CoDOLRadiant WIN32
	src/main.cpp
	src/actions.cpp
	src/assets.cpp
	src/assets.hpp
	src/asset_windows.cpp
	src/action_table.cpp
	src/compat.cpp
	src/effects.cpp
	src/ext.cpp
	src/ext.hpp
	src/hotkeys.cpp
	src/menu_keys.cpp
	src/menu_renderer.cpp
	src/menu_textures.cpp
	src/preferences.cpp
	src/toolbar.cpp
	src/tools.cpp
	src/edit3d.cpp
	src/edit3d.hpp
	src/grid_view.cpp
	src/host.hpp
	src/install.cpp
	src/packed_scripts.cpp
	src/lights.cpp
	src/maps_dialog.cpp
	src/mouse_capture.cpp
	src/mouse_capture.hpp
	src/menu_brush_patch.cpp
	src/menu_ext.hpp
	src/menu_selection.cpp
	src/filters.cpp
	src/prefabs.cpp
	src/preview.cpp
	src/preview.hpp
	src/scene.cpp
	src/scene.hpp
	src/ui.hpp
	src/viewport.cpp
	src/windows.cpp
	${DEPS}/ImGuizmo/ImGuizmo.cpp
	compat/std_include.hpp
	compat/editor_map.hpp

	# shared with the radiant plugin
	${ROOT}/src/codol/script_lex.cpp
	${ROOT}/src/codol/anim_index.cpp
	${ROOT}/src/components/fonts.cpp
	${ROOT}/src/components/modules/codol_game.cpp
	${ROOT}/src/ggui/script_workbench.cpp

	${ROOT}/codol_view/tools/png_wic.cpp)

# compat first: its std_include.hpp replaces the plugin's
target_include_directories(CoDOLRadiant PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/compat
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${ROOT}/src
	${ROOT}/codol_view/tools
	${DEPS}/ImGuizmo
	${DEPS}/rapidjson/include)

target_compile_definitions(CoDOLRadiant PRIVATE CODOL_STANDALONE=1 _WINDOWS UNICODE _UNICODE
	_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS
	$<$<CONFIG:Debug>:DEBUG _DEBUG> $<$<CONFIG:Release>:NDEBUG>)

target_compile_options(CoDOLRadiant PRIVATE /std:c++latest /W3 /MP /wd4996 /wd4267 /wd4244 $<$<CONFIG:Release>:/O2 /Zi>)
target_link_options(CoDOLRadiant PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
target_precompile_headers(CoDOLRadiant PRIVATE compat/std_include.hpp)
set_source_files_properties(${DEPS}/ImGuizmo/ImGuizmo.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON COMPILE_OPTIONS "/W0")

target_link_libraries(CoDOLRadiant PRIVATE imgui codol_edit codol_zone codol_render codol_view codol_assets
	d3d9 ws2_32 psapi windowscodecs ole32 shell32 winmm comdlg32)

# ---- the map compiler, next to the editor ( File > Compile map )
set(CODOL_MAPC_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(CODOL_MAPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CODOL_MAPC_BUILD_VIEWTOOL OFF CACHE BOOL "" FORCE)
add_subdirectory(${ROOT}/codol_mapc ${CMAKE_BINARY_DIR}/codol_mapc)
set_target_properties(codol_mapc_tool PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
add_dependencies(CoDOLRadiant codol_mapc_tool codol_assetc)

# ---- the map compiler ( from a .map alone ), next to the editor
set(CODOL_BSP_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(CODOL_BSP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${ROOT}/codol_bsp ${CMAKE_BINARY_DIR}/codol_bsp)
set_target_properties(codol_bsp_tool PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
add_dependencies(CoDOLRadiant codol_bsp_tool)

# Build map runs the packaging script ( minimap material, load zone, level scripts, list files ) from tools\ next to the exe
add_custom_command(TARGET CoDOLRadiant POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:CoDOLRadiant>/tools
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ROOT}/codol_bsp/tools/package_map.ps1 $<TARGET_FILE_DIR:CoDOLRadiant>/tools/package_map.ps1)
