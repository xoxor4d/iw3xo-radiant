#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// *
// gui
#define GET_GGUI Game::Globals::gui
#define GGUI_READY Game::Globals::gui.imgui_initialized
#define GGUI_READY_DVARS Game::Globals::gui.dvars_initialized
#define GGUI_ANY_MENUS_OPEN Game::Globals::gui.any_menus_open
#define GGUI_MENU_COUNT 2

#define IMGUI_CONTEXT_COUNT 2 // amount of imgui contexts
#define IMGUI_CONTEXT_MENUS 1 // amount of menus per context

#define IMGUI_BEGIN_CCAMERAWND ImGui::SetCurrentContext(ggui::state.ccamerawnd.context)
#define IMGUI_BEGIN_CXYWND ImGui::SetCurrentContext(ggui::state.cxywnd.context)

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
#define AssertSize(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")
#define STATIC_ASSERT_SIZE(struct, size)				static_assert(sizeof(struct) == size, "Size check")
#define STATIC_ASSERT_OFFSET(struct, member, offset)	static_assert(offsetof(struct, member) == offset, "Offset check")

#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include "common/imgui/imgui_addons.hpp"

#include "detours/Detours.h"

#include "utils/utils.hpp"
#include "utils/memory.hpp"
#include "utils/hooking.hpp"
#include "utils/function.hpp"
#include "utils/vector.hpp"

#include "common/afx.hpp"
#include "game/structs.hpp"
#include "game/functions.hpp"
#include "game/dvars.hpp"

#include "common/camwnd.hpp"
#include "common/mainframe.hpp"

#include "components/fonts.hpp"
#include "components/loader.hpp"

using namespace std::literals;