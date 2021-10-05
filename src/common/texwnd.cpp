#include "std_include.hpp"

texwnd_s* g_texwnd = reinterpret_cast<texwnd_s*>(0x25D7990);

ImGuiTextFilter	imgui_filter;
int				imgui_filter_last_len = 0;

void ctexwnd::on_mousebutton_down(UINT nFlags)
{
	auto point = ggui::get_rtt_texturewnd()->cursor_pos_pt;

	const static uint32_t CTexWnd_OnButtonDown_Func = 0x45C9A0;
	__asm
	{
		pushad;
		push	point.y;
		push	point.x;
		mov		eax, nFlags;
		call	CTexWnd_OnButtonDown_Func;
		add     esp, 8;
		popad;
	}
}

void ctexwnd::on_mousebutton_up(UINT nFlags)
{
	if ((nFlags & (MK_MBUTTON | MK_RBUTTON | MK_LBUTTON)) == 0)
	{
		// CTexWnd::OnButtonUp
		cdeclcall(void, 0x45CA30);

		ReleaseCapture();
	}
}

void ctexwnd::on_mousemove(UINT nFlags)
{
	const static uint32_t CTexWnd__OnMouseFirst_Func = 0x45CA60;
	__asm
	{
		pushad;
		mov		eax, nFlags;
		call	CTexWnd__OnMouseFirst_Func;
		popad;
	}
}

BOOL __fastcall ctexwnd::on_paint(ctexwnd* pThis)
{
	PAINTSTRUCT Paint;
	BeginPaint(pThis->GetWindow(), &Paint);

	// R_CheckHwnd_or_Device
	if (!utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(pThis->GetWindow()))
	{
		return EndPaint(pThis->GetWindow(), &Paint);
	}

	// R_BeginFrame
	cdeclcall(void, 0x4FCB10);

	// R_Clear
	utils::hook::call<void(__cdecl)(int, const float*, float, char)>(0x4FCC70)(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);

	// SetProjectionType ??
	cdeclcall(game::GfxCmdHeader*, 0x4FD390);

	// R_DrawTexWnd
	cdeclcall(void, 0x45D0F0);
	
	// R_EndFrame
	cdeclcall(void, 0x4FCBC0);

	// R_IssueRenderCommands
	utils::hook::call<void(__cdecl)(int)>(0x4FD630)(-1);

	// still no clue what this is
	cdeclcall(void, 0x4FD910);

	// R_CheckTargetWindow
	utils::hook::call<void(__cdecl)(HWND)>(0x500660)(pThis->GetWindow());

	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);

	if (!random_dword01)
	{
		__debugbreak();
	}

	random_dword03 = random_dword02;

	// nice meme IW
	return EndPaint(pThis->GetWindow(), &Paint);
}

// *
// *

//void handle_windowfocus_overlaywidget(ggui::render_to_texture_window_s* wnd)
//{
//	if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
//	{
//		wnd->window_hovered = false;
//	}
//}

void rtt_texture_window_toolbar([[maybe_unused]] ImVec2 cursor_pos)
{
	auto texwnd = ggui::get_rtt_texturewnd();
	//ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + 40.0f)); // no effect with sameline below

	// right side alignment
	static float toolbar_line1_width = 580.0f; // used as first frame estimate
	ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line1_width + 8.0f));

	// offset toolbar vertically
	ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 8.0f);

	// group all so we can get the actual toolbar width for the next frame
	ImGui::BeginGroup();
	{
		//ImGui::SameLine();
		ImGui::PushStyleCompact();
		ImGui::TextUnformatted("Usage:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(140.0f);

		if (ImGui::BeginCombo("##combo_usage", game::filter_usage_array[game::texWndGlob_usageFilter].name, ImGuiComboFlags_HeightLarge))
		{
			for (std::uint8_t i = 0; i < game::texWndGlob_usageCount; i++)
			{
				const char* name = game::filter_usage_array[i].name;
				if (name)
				{
					if (ImGui::Selectable(name, game::texWndGlob_usageFilter == i))
					{
						game::texWndGlob_usageFilter = i;
						game::g_nUpdateBits |= W_TEXTURE;
						g_texwnd->nPos = 0; // scroll to top

					} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (game::texWndGlob_usageFilter == i)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				else if (game::filter_usage_array[i].index == -1)
				{
					SEPERATORV(0.0f);
				}
			}

			ImGui::EndCombo();
		} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);
		ImGui::PopStyleCompact();

#if 0 // who needs this filter anyway
		ImGui::SameLine();
		ImGui::TextUnformatted("Locale:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(140.0f);
		if (ImGui::BeginCombo("##combo_locale", game::filter_locale_array[game::texWndGlob_localeFilter].name, ImGuiComboFlags_HeightLarge))
		{
			for (std::uint8_t i = 0; i < game::texWndGlob_localeCount; i++)
			{
				const char* name = game::filter_locale_array[i].name;
				if (name)
				{
					if (ImGui::Selectable(name, game::texWndGlob_localeFilter == i))
					{
						game::texWndGlob_localeFilter = i;
						game::g_nUpdateBits |= W_TEXTURE;
						g_texwnd->nPos = 0; // scroll to top

					} handle_windowfocus_overlaywidget(texwnd);

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (game::texWndGlob_localeFilter == i)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				else if (game::filter_locale_array[i].index == -1)
				{
					SEPERATORV(0.0f);
				}
			}

			ImGui::EndCombo();
		} handle_windowfocus_overlaywidget(texwnd);
		ImGui::PopStyleCompact();
#endif

		// ---------

		ImGui::SameLine();
		ImGui::PushStyleCompact();
		ImGui::TextUnformatted("Surface:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(140.0f);

		if (ImGui::BeginCombo("##combo_surface", game::filter_surfacetype_array[game::texWndGlob_surfaceTypeFilter].name, ImGuiComboFlags_HeightLarge))
		{
			for (std::uint8_t i = 0; i < 29; i++) // hardcoded value
			{
				const char* name = game::filter_surfacetype_array[i].name;
				if (name)
				{
					if (ImGui::Selectable(name, game::texWndGlob_surfaceTypeFilter == i))
					{
						game::texWndGlob_surfaceTypeFilter = i;
						game::g_nUpdateBits |= W_TEXTURE;
						g_texwnd->nPos = 0; // scroll to top

					} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (game::texWndGlob_surfaceTypeFilter == i)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				else if (game::filter_surfacetype_array[i].index == -1)
				{
					SEPERATORV(0.0f);
				}
			}

			ImGui::EndCombo();
		} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);
		ImGui::PopStyleCompact();

		// ---------

		ImGui::SameLine();
		ImGui::PushStyleCompact();
		ImGui::SetNextItemWidth(80.0f);
		const auto prefs = game::g_PrefsDlg();

		const std::string combo_zoom_preview = std::to_string(prefs->m_nTextureWindowScale) + "%";
		if (ImGui::BeginCombo("##combo_zoom", combo_zoom_preview.c_str(), ImGuiComboFlags_HeightLarge))
		{
			if (ImGui::Selectable("10%", prefs->m_nTextureWindowScale == 10)) {
				mainframe_thiscall(void, 0x42AFE0); // CMainFrame::OnTexturesTexturewindowscale10
			}

			if (ImGui::Selectable("25%", prefs->m_nTextureWindowScale == 25)) {
				mainframe_thiscall(void, 0x42B040); // CMainFrame::OnTexturesTexturewindowscale25
			}

			if (ImGui::Selectable("50%", prefs->m_nTextureWindowScale == 50)) {
				mainframe_thiscall(void, 0x42B060); // CMainFrame::OnTexturesTexturewindowscale50
			}

			if (ImGui::Selectable("100%", prefs->m_nTextureWindowScale == 100)) {
				mainframe_thiscall(void, 0x42B000); // CMainFrame::OnTexturesTexturewindowscale100
			}

			if (ImGui::Selectable("200%", prefs->m_nTextureWindowScale == 200)) {
				mainframe_thiscall(void, 0x42B020); // CMainFrame::OnTexturesTexturewindowscale200
			}

			ImGui::EndCombo();
		} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);
		ImGui::PopStyleCompact();

#if 0
		ImGui::Text("Texwnd hovered? %d", texwnd->window_hovered);
		ImGui::Text("nPos: %d", g_texwnd->nPos);
		ImGui::Text("needRange: %d", g_texwnd->m_bNeedRange);
		ImGui::Text("textureoffset: %d", g_texwnd->textureOffset);
#endif

		ImGui::EndGroup();
	}
	
	toolbar_line1_width = ImGui::GetItemRectSize().x; // save width for the next frame

	
	// *
	// line 2
	
	static float toolbar_line2_width = 220.0f; // used as first frame estimate
	ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line2_width + 8.0f));

	// offset toolbar vertically
	ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 28.0f);

	ImGui::BeginGroup();
	{
		const auto pre_filter_pos = ImGui::GetCursorScreenPos();

		ImGui::PushStyleCompact();
		imgui_filter.Draw("##texture_filter", 230.0f);

		if (!imgui_filter.IsActive())
		{
			ImGui::SetCursorScreenPos(ImVec2(pre_filter_pos.x + 12.0f, pre_filter_pos.y + 2.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
			ImGui::TextUnformatted("Search ..");
			ImGui::PopStyleColor();

			g_texwnd->searchbar_filter = false;
			g_texwnd->searchbar_buffer = "";
			imgui_filter_last_len = 0;
		}
		else
		{
			const int curr_len = strlen(imgui_filter.InputBuf);
			if (imgui_filter_last_len != curr_len)
			{
				g_texwnd->nPos = 0; // scroll to top
			}

			g_texwnd->searchbar_filter = true;
			g_texwnd->searchbar_buffer = imgui_filter.InputBuf;

			// buffer length is located -12bytes from searchbar_buffer
			*(int*)(g_texwnd->searchbar_buffer - 3 * sizeof(int)) = curr_len;
			imgui_filter_last_len = curr_len;
		}

		ImGui::EndGroup();
	} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);

	toolbar_line2_width = ImGui::GetItemRectSize().x; // save width for the next frame
	ImGui::PopStyleCompact();
}


// gui::render_loop()
// render to texture - imgui texture window
void ctexwnd::rtt_texture_window()
{
	int p_styles = 0;
	int p_colors = 0;

	const auto prefs = game::g_PrefsDlg();
	const auto texture_size = ImVec2(static_cast<float>(g_texwnd->m_nWidth), static_cast<float>(g_texwnd->m_nHeight));

	if (prefs->m_bTextureScrollbar)
	{
		const auto tex_hwnd = cmainframe::activewnd->m_pTexWnd->GetWindow();
		prefs->m_bTextureScrollbar = false;
		ShowScrollBar(tex_hwnd, 1, 0);
		InvalidateRect(tex_hwnd, 0, 1);
		UpdateWindow(tex_hwnd);
		//scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); p_styles++;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;

	auto texwnd = ggui::get_rtt_texturewnd();

	if (texwnd->should_set_focus)
	{
		ImGui::SetNextWindowFocus();
		texwnd->should_set_focus = false;
	}

	if(!ImGui::Begin("Textures##rtt", &texwnd->menustate, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
		return;
	}
	
	if (texwnd->scene_texture)
	{
		bool tabbar_visible = true;
		const auto wnd = ImGui::GetCurrentWindow();

		if (ImGui::IsWindowDocked() && wnd)
		{
			if (wnd->DockNode && wnd->DockNode->IsHiddenTabBar())
			{
				tabbar_visible = false;
			}
		}

		const float frame_height = tabbar_visible ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
		
		const auto IO = ImGui::GetIO();
		texwnd->scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, frame_height);

		const auto pre_child_pos = ImGui::GetCursorScreenPos();

		// hack to disable left mouse window movement
		ImGui::BeginChild("scene_child", ImVec2(texture_size.x, texture_size.y + frame_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			const auto screenpos = ImGui::GetCursorScreenPos();
			SetWindowPos(cmainframe::activewnd->m_pTexWnd->GetWindow(), HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)texwnd->scene_size_imgui.x, (int)texwnd->scene_size_imgui.y, SWP_NOZORDER);
			const auto pre_image_cursor = ImGui::GetCursorPos();

			ImGui::Image(texwnd->scene_texture, texture_size);
			texwnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

			// pop ItemSpacing
			ImGui::PopStyleVar(); p_styles--;

			ImGui::SetCursorPos(pre_image_cursor);
			const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

			texwnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
			texwnd->cursor_pos_pt = CPoint((LONG)texwnd->cursor_pos.x, (LONG)texwnd->cursor_pos.y);

			// overlay toolbar
			rtt_texture_window_toolbar(pre_child_pos);

			// reset cursorpos and fix tabbar triangle
			ImGui::SetCursorScreenPos(pre_child_pos);
			ggui::FixDockingTabbarTriangle(wnd, texwnd);
			
			ImGui::EndChild();
		}
	}
	
	ImGui::PopStyleColor(p_colors);
	ImGui::PopStyleVar(p_styles);
	ImGui::End();
}


// *
// *

// CMainFrame::OnViewTexture
void on_viewtextures_command()
{
	components::gui::toggle(ggui::get_rtt_texturewnd(), 0, true);
}

void ctexwnd::hooks()
{
	// do not set parent window for texture window
	utils::hook::nop(0x4228C1, 8);

	// ignore crashing texture searchbar destructor :x
	utils::hook::nop(0x627990, 5);
	utils::hook::set<BYTE>(0x627990, 0xC3);

	// disable texture tab insertion in entitywnd :: CTabCtrl::InsertItem(&g_wndTabsEntWnd, 1u, 1u, "&Textures", 0, 0);
	utils::hook::nop(0x49672A, 23);

	// TODO! :: why does the default OnPaint function induces lag on all windows (even outside radiant) calling it at 250fps
	// -- rewritten one runs fine
	utils::hook::detour(0x45DB20, ctexwnd::on_paint, HK_JUMP);

	// detour CMainFrame::OnViewTexture (hotkey to open the original dialog)
	utils::hook::detour(0x424440, on_viewtextures_command, HK_JUMP);
}