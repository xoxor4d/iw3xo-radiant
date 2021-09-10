#pragma once
#include "imgui_internal.h"

namespace ImGui
{
	ImGuiID FindNodeByID(ImGuiID id);
	void	DockBuilderDockWindow_FirstUseOrSaved(const char* window_name, ImGuiID node_id);
	bool	IsItemHoveredDelay(float delay_in_seconds);
	void	HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta = 0, UINT nChar = 0);
	ImVec4	ToImVec4(const float* in);
	void	HelpMarker(const char* desc);
	void	PushFontFromIndex(int font);
	void	StyleColorsDevgui(ImGuiStyle* = NULL);
	
}
