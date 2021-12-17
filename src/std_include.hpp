#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// *
// gui
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_BEGIN_CZWND if(ggui::cz_context_ready()) ImGui::SetCurrentContext(ggui::state.czwnd.context)

// Version number
#include <version.hpp>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>
#include <wincrypt.h>
#include <time.h>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <d3d9.h>
#include <d3dx9.h>
#pragma warning(pop)

#include <timeapi.h>
#include <shellapi.h>
#include <WinSock2.h>
#include <assert.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

#pragma warning(push)
#pragma warning(disable: 4244)
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <regex>
#include <thread>
#include <chrono>
#include <future>
#include <unordered_map>
#include <iostream>
#pragma warning(pop)

#define ASSERT_MSG(expr, msg) assert((msg, expr))
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define AssertSize(x, size)								static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")
#define STATIC_ASSERT_SIZE(struct, size)				static_assert(sizeof(struct) == size, "Size check")
#define STATIC_ASSERT_OFFSET(struct, member, offset)	static_assert(offsetof(struct, member) == offset, "Offset check")

#pragma warning(push)
#pragma warning(disable: 6011)
#pragma warning(disable: 28182)
#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>
#pragma warning(pop)

#include "common/imgui/imgui_addons.hpp"
#include "detours/Detours.h"
#include "fifo_map.hpp"

#pragma warning(push, 0)  
#include "ImGuizmo.h"
#pragma warning(pop)

#include "game/structs.hpp"
#include "utils/utils.hpp"
#include "utils/memory.hpp"
#include "utils/hooking.hpp"
#include "utils/function.hpp"
#include "utils/vector.hpp"

#include "common/afx.hpp"
#include "game/functions.hpp"
#include "game/dvars.hpp"

#include "ggui/_ggui.hpp"
#include "ggui/hotkeys.hpp"
#include "ggui/menubar.hpp"
#include "ggui/colors.hpp"
#include "ggui/toolbar.hpp"
#include "ggui/console.hpp"
#include "ggui/filter.hpp"
#include "ggui/entity.hpp"
#include "ggui/preferences.hpp"
#include "ggui/modelselector.hpp"
#include "ggui/about.hpp"
#include "ggui/fakesun_settings.hpp"

#include "common/mainframe.hpp"
#include "common/camwnd.hpp"
#include "common/radiantapp.hpp"
#include "common/layermatwnd.hpp"
#include "common/zwnd.hpp"

#include "components/fonts.hpp"
#include "components/loader.hpp"

using namespace std::literals;
