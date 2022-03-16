#include "std_include.hpp"

namespace ggui::textures
{
	ImGuiTextFilter	imgui_filter;
	int				imgui_filter_last_len = 0;

	void toolbar()
	{
		const float RIGHT_OFFSET = 14.0f;

		const auto texwnd = ggui::get_rtt_texturewnd();
		//ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + 40.0f)); // no effect with sameline below

		// right side alignment
		static float toolbar_line1_width = 580.0f; // used as first frame estimate
		ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line1_width + RIGHT_OFFSET));

		// offset toolbar vertically
		ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 8.0f);

		// group all so we can get the actual toolbar width for the next frame
		ImGui::BeginGroup();
		{
			ImGui::PushStyleCompact();
			if (ImGui::Button("A"))
			{
				// Texture_ShowAll
				cdeclcall(void, 0x45B730);
			} ggui::rtt_handle_windowfocus_overlaywidget(texwnd); TT("Show all textures");

			ImGui::SameLine();
			if (ImGui::Button("U"))
			{
				// Texture_ShowInuse
				cdeclcall(void, 0x45B850);
			} ggui::rtt_handle_windowfocus_overlaywidget(texwnd); TT("Show all textures in use");
			ImGui::PopStyleCompact();

			ImGui::SameLine();
			ImGui::PushStyleCompact();
			//ImGui::TextUnformatted("Usage:");
			//ImGui::SameLine();
			ImGui::SetNextItemWidth(140.0f);

			const char* combo_usage_preview_str = "Usage: all";
			if (game::texWndGlob_usageFilter)
			{
				combo_usage_preview_str = game::filter_usage_array[game::texWndGlob_usageFilter].name;
			}

			if (ImGui::BeginCombo("##combo_usage", combo_usage_preview_str, ImGuiComboFlags_HeightLarge))
			{
				for (std::uint8_t i = 0; i < static_cast<std::uint8_t>(game::texWndGlob_usageCount); i++)
				{
					if (const char* name = game::filter_usage_array[i].name;
						name)
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
			//ImGui::TextUnformatted("Surface:");
			//ImGui::SameLine();
			ImGui::SetNextItemWidth(140.0f);

			const char* combo_surface_preview_str = "Surface: all";
			if (game::texWndGlob_surfaceTypeFilter)
			{
				combo_surface_preview_str = game::filter_surfacetype_array[game::texWndGlob_surfaceTypeFilter].name;
			}

			if (ImGui::BeginCombo("##combo_surface", combo_surface_preview_str, ImGuiComboFlags_HeightLarge))
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
		ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line2_width + RIGHT_OFFSET));

		// offset toolbar vertically
		ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 28.0f);

		ImGui::BeginGroup();
		{
			ImGui::PushStyleCompact();

			if (dvars::gui_texwnd_draw_scrollpercent && dvars::gui_texwnd_draw_scrollpercent->current.enabled)
			{
				const int scroll_percent = (int)((float)(g_texwnd->nPos) / (float)(g_texwnd->nPosMax - g_texwnd->m_nHeight) * 100);
				if (scroll_percent == 100 || g_texwnd->nPos < g_texwnd->nPosMax - g_texwnd->m_nHeight)
				{
					ImGui::Text("%d/100%%", scroll_percent);
					ImGui::SameLine();
				}
			}

			const auto pre_filter_pos = ImGui::GetCursorScreenPos();
			imgui_filter.Draw("##texture_filter", 230.0f);

			if (!imgui_filter.IsActive())
			{
				ImGui::SetCursorScreenPos(ImVec2(pre_filter_pos.x + 12.0f, pre_filter_pos.y + 2.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
				ImGui::TextUnformatted("Search ..");
				ImGui::PopStyleColor();

				g_texwnd->searchbar_filter = false;
				//g_texwnd->searchbar_buffer = "";
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
				imgui_filter_last_len = curr_len;
			}


			ImGui::EndGroup();
		} ggui::rtt_handle_windowfocus_overlaywidget(texwnd);

		toolbar_line2_width = ImGui::GetItemRectSize().x; // save width for the next frame
		ImGui::PopStyleCompact();
	}

	// gui::render_loop()
	// render to texture - imgui texture window

	void gui()
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
			InvalidateRect(tex_hwnd, nullptr, 1);
			UpdateWindow(tex_hwnd);
			//scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
		}

		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 320.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;

		auto texwnd = ggui::get_rtt_texturewnd();

		// yes 2 bools for the same thing, cba to refactor and test use-cases across all menus
		if (texwnd->should_set_focus || texwnd->bring_tab_to_front)
		{
			texwnd->bring_tab_to_front = false;
			texwnd->should_set_focus = false;
			ImGui::SetNextWindowFocus();
		}

		if (!ImGui::Begin("Textures##rtt", &texwnd->menustate, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			texwnd->inactive_tab = true;
			ImGui::PopStyleColor(p_colors);
			ImGui::PopStyleVar(p_styles);
			ImGui::End();
			return;
		}

		texwnd->inactive_tab = false;

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
				texwnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

				texwnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
				texwnd->cursor_pos_pt = CPoint((LONG)texwnd->cursor_pos.x, (LONG)texwnd->cursor_pos.y);

				// overlay toolbar
				toolbar();

				// reset cursorpos and fix tabbar triangle
				ImGui::SetCursorScreenPos(pre_child_pos);
				ggui::FixDockingTabbarTriangle(wnd, texwnd);


				// *
				// custom scrollbar
				if (dvars::gui_texwnd_draw_scrollbar && dvars::gui_texwnd_draw_scrollbar->current.enabled)
				{
					ImGuiContext& g = *GImGui;
					ImGuiWindow* window = g.CurrentWindow;

					const ImGuiID id = ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y);
					ImGui::KeepAliveID(id);

					// Calculate scrollbar bounding box
					const ImRect outer_rect = window->Rect();
					const ImRect inner_rect = window->InnerRect;
					const float border_size = window->WindowBorderSize;
					const float scrollbar_size = 10.0f;

					const ImRect bb = ImRect(ImMax(outer_rect.Min.x, outer_rect.Max.x - border_size - scrollbar_size), inner_rect.Min.y, outer_rect.Max.x, inner_rect.Max.y);
					const ImDrawFlags rounding_corners = ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight;

					ImS64 scroll = g_texwnd->nPos;
					const float size_avail = window->InnerRect.Max[ImGuiAxis_Y] - window->InnerRect.Min[ImGuiAxis_Y];
					const float size_contents = (float)g_texwnd->nPosMax + 110.0f;

					ImGui::ScrollbarEx(bb, id, ImGuiAxis_Y, &scroll, static_cast<ImS64>(size_avail), static_cast<ImS64>(size_contents), rounding_corners);

					g_texwnd->nPos = static_cast<int>(scroll);
				}

				ImGui::EndChild();
			}
		}

		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
	}
}