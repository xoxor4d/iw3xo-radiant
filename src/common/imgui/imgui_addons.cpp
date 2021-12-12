#include "std_include.hpp"
#include "imgui_internal.h"

namespace ImGui
{
	ImGuiID FindNodeByID(ImGuiID id)
	{
		if(const auto node = (ImGuiDockNode*)GImGui->DockContext.Nodes.GetVoidPtr(id);
					  node)
		{
			return node->ID;
		}
		
		return 0;
	}
	
	void DockBuilderDockWindow_FirstUseOrSaved(const char* window_name, ImGuiID node_id)
	{
		ImGuiID window_id = ImHashStr(window_name);
		ImGuiWindowSettings* settings = FindWindowSettings(window_id);
		
		if (ImGuiWindow* window = FindWindowByID(window_id))
		{
			if(settings)
			{
				SetWindowDock(window, settings->DockId, ImGuiCond_FirstUseEver);
				window->DockOrder = settings->DockOrder;
			}
			else
			{
				// Apply to created window
				SetWindowDock(window, node_id, ImGuiCond_FirstUseEver);
				window->DockOrder = -1;
			}
		}
		else
		{
			if (settings == nullptr)
			{
				settings = CreateNewWindowSettings(window_name);
				settings->DockId = node_id;
				settings->DockOrder = -1;
			}
		}
	}

	void PushStyleCompact()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
	}

	void PopStyleCompact()
	{
		ImGui::PopStyleVar(2);
	}
	
	bool IsItemHoveredDelay(float delay_in_seconds)
	{
		ImGuiContext& g = *GImGui;

		if (ImGui::IsItemHovered() && !g.IO.MouseDown[0] && GImGui->HoveredIdTimer > delay_in_seconds)
		{
			return true;
		}

		return false;
	}

	bool IsResizing()
	{
		switch (ImGui::GetMouseCursor())
		{
		case ImGuiMouseCursor_ResizeAll:
		case ImGuiMouseCursor_ResizeNS:
		case ImGuiMouseCursor_ResizeEW:
		case ImGuiMouseCursor_ResizeNESW:
		case ImGuiMouseCursor_ResizeNWSE:
			return true;
		}

		return false;
	}

	// "custom" ImGui_ImplWin32_WndProcHandler
	// * hook a wndclass::function handling input and call this function with the corrosponding WM_ msg
	void HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta, UINT nChar)
	{
		if (ImGui::GetCurrentContext() == NULL)
			return;

		ImGuiIO& io = ImGui::GetIO();

		switch (key)
		{
		case WM_SETFOCUS:
			std::fill_n(io.KeysDown, 512, 0);
			return;

		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		{
			int button = 0;
			if (key == WM_LBUTTONDOWN || key == WM_LBUTTONDBLCLK) { button = 0; }
			if (key == WM_RBUTTONDOWN || key == WM_RBUTTONDBLCLK) { button = 1; }
			if (key == WM_MBUTTONDOWN || key == WM_MBUTTONDBLCLK) { button = 2; }
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == nullptr)
				::SetCapture(hwnd);
			io.MouseDown[button] = true;
			return;
		}

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (key == WM_LBUTTONUP) { button = 0; }
			if (key == WM_RBUTTONUP) { button = 1; }
			if (key == WM_MBUTTONUP) { button = 2; }
			io.MouseDown[button] = false;
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
				::ReleaseCapture();
			return;
		}

		case WM_MOUSEWHEEL:
			io.MouseWheel += static_cast<float>(zDelta) / 120.0f; // WHEEL_DELTA
			return;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (nChar < 256)
				io.KeysDown[nChar] = true;
			return;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (nChar < 256)
				io.KeysDown[nChar] = false;
			return;
		}
	}

	ImVec4 ToImVec4(const float* in)
	{
		if(in)
		{
			return ImVec4(in[0], in[1], in[2], in[3]);
		}

		return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void PushFontFromIndex(int font)
	{
		ImGuiIO& io = ImGui::GetIO();

		if(io.Fonts->Fonts[font])
		{
			ImGui::PushFont(io.Fonts->Fonts[font]);
		}
		else
		{
			ImGui::PushFont(ImGui::GetDefaultFont());
		}
	}

	void AddUnterline(ImColor col)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col, 1.0f);
	}

	void TextURL(const char* name, const char* url)
	{
		ImGui::TextUnformatted(name);

		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
				ShellExecuteA(0, 0, url, 0, 0, SW_SHOW);
			}

			AddUnterline(ImGui::GetStyle().Colors[ImGuiCol_TabHovered]);
			ImGui::SetTooltip("Open in browser\n[%s]", url);
		}
		else
		{
			AddUnterline(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
	}

	void SetCursorForCenteredText(const char* text)
	{
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) * 0.5f);
	}
	
	void debug_table_entry_vec3(const char* label, const float* vec3)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		for (auto i = 0; i < 3; i++)
		{
			ImGui::TableNextColumn();
			ImGui::Text("%.2f", vec3[i]);
		}
	}

	void debug_table_entry_float(const char* label, const float* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		ImGui::TableNextColumn();
		ImGui::Text("%.2f", val);
	}

	void debug_table_entry_int(const char* label, const int val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		ImGui::TableNextColumn();
		ImGui::Text("%d", val);
	}

	void debug_table_entry_int2(const char* label, const int* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		for (auto i = 0; i < 2; i++)
		{
			ImGui::TableNextColumn();
			ImGui::Text("%d", val[i]);
		}
	}

	void StyleColorsDevgui(ImGuiStyle* dst)
	{
		ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		style->AntiAliasedLines = true;
		style->AntiAliasedFill = true;

		// Main
		style->DisplaySafeAreaPadding.y = 18.0f; // fix child menus going out of view
		style->WindowPadding.x = 5.0f;
		style->WindowPadding.y = 5.0f;
		style->PopupRounding = 2.0f;
		style->FramePadding.x = 10.0f;
		style->FramePadding.y = 5.0f;
		style->ItemSpacing.x = 10.0f;
		style->ItemSpacing.y = 5.0f;
		style->ItemInnerSpacing.x = 10.0f;
		style->ItemInnerSpacing.y = 4.0f;
		style->TouchExtraPadding.x = 0.0f;
		style->TouchExtraPadding.y = 0.0f;
		style->IndentSpacing = 30.0f;
		style->ScrollbarSize = 14.0f;
		style->GrabMinSize = 10.0f;

		// Borders
		style->WindowBorderSize = 1.0f;
		style->ChildBorderSize = 1.0f;
		style->PopupBorderSize = 1.0f;
		style->FrameBorderSize = 1.0f;
		style->TabBorderSize = 0.0f;

		// Rounding
		style->WindowRounding = 2.0f;
		style->ChildRounding = 2.0f;

		// 04.10.21
		//style->FrameRounding = 2.0f;
		style->FrameRounding = 5.0f;
		
		style->ScrollbarRounding = 2.0f;
		style->GrabRounding = 2.0f;

		// 04.10.21
		//style->TabRounding = 2.0f;	
		style->TabRounding = 6.0f;
		
		// Alignment
		style->WindowTitleAlign.x = 0.5f;
		style->WindowTitleAlign.y = 0.5f;
		style->Alpha = 1.0f;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.84f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 0.64f);

		// 04.10.21
		//colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.82f);						
		colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);

		// 04.10.21
		//colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);						
		colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);	
		
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.93f);
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.55f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.44f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);

		// 04.10.21
		//colors[ImGuiCol_TitleBg] = ImVec4(0.30f, 0.01f, 0.02f, 0.35f);						
		//colors[ImGuiCol_TitleBgActive] = ImVec4(0.30f, 0.01f, 0.02f, 0.51f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.66f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.11f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.24f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.99f, 1.00f, 1.00f, 0.24f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);

		//colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		//colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.59f, 0.59f, 0.21f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		
		colors[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.24f, 0.55f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.73f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.55f, 0.55f, 0.74f);
		colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.21f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.36f);
		colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.36f);

		// 04.10.21
		//colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);						
		colors[ImGuiCol_Separator] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		
		colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);

		// 04.10.21
		//colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		//colors[ImGuiCol_TabHovered] = ImVec4(0.52f, 0.01f, 0.02f, 0.63f);
		//colors[ImGuiCol_TabActive] = ImVec4(0.52f, 0.01f, 0.02f, 0.45f);
		//colors[ImGuiCol_TabUnfocused] = ImVec4(0.42f, 0.00f, 0.02f, 0.00f);
		//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.52f, 0.01f, 0.02f, 0.45f);
		colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.17f, 0.17f, 0.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.49f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.49f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.17f, 0.17f, 0.17f, 0.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
		
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.70f, 0.62f, 0.30f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.27f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.34f);
	}
}
