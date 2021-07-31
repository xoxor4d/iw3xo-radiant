#pragma once
#include "imgui_internal.h"

namespace ImGui
{
	bool IsItemHoveredDelay(float delay_in_seconds);
	void HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta = 0, UINT nChar = 0);
	void StyleColorsDevgui(ImGuiStyle* = NULL);

}
