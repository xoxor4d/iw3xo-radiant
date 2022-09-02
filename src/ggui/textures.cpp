#include "std_include.hpp"

namespace ggui
{
	void texture_dialog::toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 10.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

		const float searchbar_spacing = 8.0f;
		const float searchbar_width = 220.0f;
		const auto pre_searchbar_pos_screen = ImGui::GetCursorScreenPos();
		imgui_filter.Draw("##texture_filter", searchbar_width);
		
		if (!imgui_filter.IsActive())
		{
			ImGui::SetCursorScreenPos(ImVec2(pre_searchbar_pos_screen.x + 12.0f, pre_searchbar_pos_screen.y + 4.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
			ImGui::TextUnformatted("Search ..");
			ImGui::PopStyleColor();
			ImGui::SetCursorScreenPos(ImVec2(pre_searchbar_pos_screen.x + searchbar_width + searchbar_spacing, pre_searchbar_pos_screen.y));

			g_texwnd->searchbar_filter = false;
			imgui_filter_last_len = 0;
		}
		else
		{
			const int curr_len = static_cast<int>(strlen(imgui_filter.InputBuf));
			if (imgui_filter_last_len != curr_len)
			{
				g_texwnd->nPos[0].nPos_current = 0; // scroll to top
			}

			g_texwnd->searchbar_filter = true;
			imgui_filter_last_len = curr_len;

			ImGui::SameLine(0, searchbar_spacing);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 0.65f)));
		if (ImGui::Button("All"))
		{
			// Texture_ShowAll
			cdeclcall(void, 0x45B730);
		} TT("Show all textures");

		ImGui::SameLine(0.0f, searchbar_spacing);
		if (ImGui::Button("Use"))
		{
			// Texture_ShowInuse
			cdeclcall(void, 0x45B850);
		} TT("Show all textures in use");

		ImGui::PopStyleColor();

		ImGui::SameLine(0.0f, searchbar_spacing);
		ImGui::SetNextItemWidth(120.0f);

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
						g_texwnd->nPos[0].nPos_current = 0; // scroll to top

					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

					// set the initial focus when opening the combo (scrolling + keyboard navigation focus)
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
		} TT("Filter textures by usage");

		// ---------

		ImGui::SameLine(0.0f, searchbar_spacing);
		ImGui::SetNextItemWidth(120.0f);

		const char* combo_surface_preview_str = "Surface: all";
		if (game::texWndGlob_surfaceTypeFilter)
		{
			combo_surface_preview_str = game::filter_surfacetype_array[game::texWndGlob_surfaceTypeFilter].name;
		}

		if (ImGui::BeginCombo("##combo_surface", combo_surface_preview_str, ImGuiComboFlags_HeightLarge))
		{
			for (std::uint8_t i = 0; i < 29; i++) // hardcoded value
			{
				if (const char* name = game::filter_surfacetype_array[i].name;
					name)
				{
					if (ImGui::Selectable(name, game::texWndGlob_surfaceTypeFilter == i))
					{
						game::texWndGlob_surfaceTypeFilter = i;
						game::g_nUpdateBits |= W_TEXTURE;
						g_texwnd->nPos[0].nPos_current = 0; // scroll to top

					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

					// set the initial focus when opening the combo (scrolling + keyboard navigation focus)
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
		} TT("Filter textures by surfacetype");


		// ---------

		ImGui::SameLine(0.0f, searchbar_spacing);
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
		} TT("Texture display scale");

		ImGui::SameLine(0.0f, searchbar_spacing);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine(0.0f, searchbar_spacing);

		ImGui::SetNextItemWidth(140.0f);

		const char* combo_favourites_preview_str = texwnd_vector_of_favourites.empty() ? "Favourite: none" : !get_favourite_index() ? "Favourite: none" : get_favourite_str().c_str();

		if (ImGui::BeginCombo("##combo_favourites", combo_favourites_preview_str, ImGuiComboFlags_HeightLarge))
		{
			if (ImGui::Selectable("none", !get_favourite_index()))
			{
				set_favourite_index(0);
				set_favourite_str("none");
				g_texwnd->nPos[0].nPos_current = 0; // scroll to top

				// Texture_ShowInuse
				cdeclcall(void, 0x45B850);
			}

			if (!get_favourite_index())
			{
				ImGui::SetItemDefaultFocus();
			}

			int fav_idx = 1;
			for (const auto& vec : texwnd_vector_of_favourites)
			{
				if (ImGui::Selectable(vec[0].c_str(), get_favourite_index() == fav_idx))
				{
					set_favourite_index(fav_idx);
					set_favourite_str(vec[0]);

					g_texwnd->nPos[0].nPos_current = 0; // scroll to top

					ctexwnd::apply_favourite(get_favourite_index() - 1);
				}

				// set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (get_favourite_index() == fav_idx)
				{
					ImGui::SetItemDefaultFocus();
				}

				fav_idx++;
			}

			ImGui::EndCombo();
		}
		TT(	"Custom favourite lists. Add new lists by using the '+' button\n"
			"Use the right-click context menu to assign textures.");

		ImGui::SameLine(0.0f, searchbar_spacing);

		if (ImGui::Button("+"))
		{
			//ImGui::OpenPopup("new_mat_favourite_list");
			m_open_new_favourite_popup = true;

		}
		TT(	"Create a new favourite list\n"
			"Use the right-click context menu to assign textures.");

#if 0
		if(!texwnd_vector_of_favourites.empty())
		{
			ImGui::SameLine(0.0f, searchbar_spacing);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine(0.0f, searchbar_spacing);

			if(m_add_selected_to_list_str.empty())
			{
				m_add_selected_to_list_str = texwnd_vector_of_favourites[0][0];
			}

			ImGui::SameLine(0.0f, searchbar_spacing);

			if (ImGui::Button("Add to:"))
			{
				ctexwnd::add_selected_to_favourite_list(m_add_to_favourites_index);
				ctexwnd::write_favourite_list(m_add_to_favourites_index);
			} TT("Adds selected texture to selected favourite list");

			ImGui::SameLine(0.0f, searchbar_spacing);

			ImGui::BeginDisabled(!get_favourite_index());
			{
				if (ImGui::Button("Del from:"))
				{
					ctexwnd::remove_selected_from_favourite_list(m_add_to_favourites_index);
					ctexwnd::write_favourite_list(m_add_to_favourites_index);

					if (get_favourite_index())
					{
						ctexwnd::apply_favourite(get_favourite_index() - 1);
					}
				} TT("Deletes selected texture from selected favourite list");

				ImGui::EndDisabled();
			}
			
			ImGui::SameLine(0.0f, searchbar_spacing);
			ImGui::SetNextItemWidth(140.0f);

			if (ImGui::BeginCombo("##combo_add_to_favourites", m_add_selected_to_list_str.c_str(), ImGuiComboFlags_HeightLarge))
			{
				int fav_idx = 0;
				for (const auto& vec : texwnd_vector_of_favourites)
				{
					if (ImGui::Selectable(vec[0].c_str(), m_add_to_favourites_index == fav_idx))
					{
						m_add_to_favourites_index = fav_idx;
						m_add_selected_to_list_str = vec[0];
					}

					if (m_add_to_favourites_index == fav_idx)
					{
						ImGui::SetItemDefaultFocus();
					}

					fav_idx++;
				}

				ImGui::EndCombo();
			} TT("The favourite list to add textures to using the button to the left");
		}
#endif

		ImGui::PopStyleVar(2);
	}

	

	// right click context menu
	void texture_dialog::context_menu()
	{
		ggui::context_menu_style_begin();

		const int  CONTEXT_DELAY = 20;
		static int context_menu_delay = 0;
		if (g_texwnd->m_was_mouse_dragged)
		{
			context_menu_delay = 0;
		}
		else if (context_menu_delay < CONTEXT_DELAY)
		{
			context_menu_delay++;
		}

		if (context_menu_delay >= CONTEXT_DELAY && ImGui::BeginPopupContextItem("context_menu##textures"))
		{
			// select texture with 
			if (imgui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ctexwnd::select_texture(GET_GUI(ggui::texture_dialog)->rtt_get_cursor_pos_cpoint());
			}

			if (const auto	mat_name = ctexwnd::get_name_for_selection();
							mat_name)
			{
				imgui::PushFontFromIndex(BOLD_17PX);
				imgui::TextUnformatted(mat_name);
				imgui::PopFont();
				imgui::Separator();

				if (!texwnd_vector_of_favourites.empty())
				{
					if (imgui::BeginMenu("Add to favourites"))
					{
						int count = 0;
						int fav_idx = 0;

						for (const auto& vec : texwnd_vector_of_favourites)
						{
							if (fav_idx != get_favourite_index() - 1)
							{
								if (ImGui::MenuItem(vec[0].c_str()))
								{
									ctexwnd::add_selected_to_favourite_list(fav_idx);
									ctexwnd::write_favourite_list(fav_idx);
								}

								count++;
							}

							fav_idx++;
						}

						if (!count)
						{
							imgui::TextUnformatted("none");
						}

						imgui::EndMenu();
					}


					if (imgui::BeginMenu("Delete from favourites"))
					{
						int count = 0;

						for (auto l = 0; l < static_cast<int>(texwnd_vector_of_favourites.size()); l++)
						{
							for (const auto& str : texwnd_vector_of_favourites[l])
							{
								if (utils::string_equals(mat_name, str.c_str()))
								{
									if (ImGui::MenuItem(texwnd_vector_of_favourites[l][0].c_str()))
									{
										ctexwnd::remove_selected_from_favourite_list(l);
										ctexwnd::write_favourite_list(l);

										if (l == get_favourite_index() - 1)
										{
											ctexwnd::apply_favourite(get_favourite_index() - 1);
										}
									}

									count++;
								}
							}
						}

						if (!count)
						{
							imgui::TextUnformatted("none");
						}

						imgui::EndMenu();
					}
				}

				imgui::Separator();

				if (ImGui::MenuItem("Create new favourite list"))
				{
					m_open_new_favourite_popup = true;
				}
			}
			
			ImGui::EndPopup();
		}

		ggui::context_menu_style_end();
	}

	void texture_dialog::popup_new_favourite_list()
	{
		if (m_open_new_favourite_popup)
		{
			ImGui::OpenPopup("new_mat_favourite_list");
			m_open_new_favourite_popup = false;
		}

		// # new fav list creation popup

		ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 5.0f));
		if (ImGui::BeginPopup("new_mat_favourite_list", ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoMove))
		{
			std::string old_fav_list_name;

			if (int i = get_favourite_index(); i && i - 1 < static_cast<int>(texwnd_vector_of_favourites.size()))
			{
				old_fav_list_name = texwnd_vector_of_favourites[i - 1][0];
			}

			const ImGuiStyle& style = ImGui::GetStyle();

			static float label_width = 120.0f;
			ImGui::PushFontFromIndex(ggui::BOLD_18PX);
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - label_width + style.FramePadding.x) * 0.5f);
			ImGui::TextUnformatted("Enter a name for the new list");

			label_width = ImGui::GetItemRectSize().x;
			ImGui::PopFont();
			ImGui::Spacing();

			static char new_list_name[500] = "";
			static char new_list_error[500] = "";

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_Button));

			bool pressed_enter = false;
			if (ImGui::InputText("##newfolder", new_list_name, sizeof(new_list_name), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				pressed_enter = true;
			}

			ImGui::PopStyleColor();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16);

			static float modal_button_group_width = 100.0f;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() - modal_button_group_width + style.FramePadding.x) * 0.5f);
			ImGui::BeginGroup();
			{
				if (ImGui::Button("Create##1") || pressed_enter)
				{
					if (strlen(new_list_name) <= 0)
					{
						strcpy_s(new_list_error, "list name can't be empty");
					}
					else
					{
						if (dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
							dvars::fs_homepath)
						{
							std::ifstream file;

							std::string dir_path = dvars::fs_homepath->current.string;
							dir_path += R"(\IW3xRadiant\texture_favourites\)";

							std::filesystem::create_directories(dir_path);

							if (std::filesystem::exists(dir_path))
							{
								std::ofstream ofile;

								const std::string file_path = dir_path + new_list_name + ".txt";

								ofile.open(file_path);

								if (ofile.is_open())
								{
									ofile << "// texture favourites generated by iw3xo-radiant" << std::endl;
									ofile << new_list_name << std::endl;

									ofile.close();
								}

								ctexwnd::load_favourites();

								if (!old_fav_list_name.empty())
								{
									int list_id = 0;
									for (const auto& list : texwnd_vector_of_favourites)
									{
										if (list[0] == old_fav_list_name)
										{
											set_favourite_index(list_id + 1);
											set_favourite_str(old_fav_list_name);
											ctexwnd::apply_favourite(list_id);
											break;
										}

										list_id++;
									}
								}
							}
						}

						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel##1"))
				{
					strcpy_s(new_list_name, "");
					strcpy_s(new_list_error, "");
					ImGui::CloseCurrentPopup();
				}

				ImGui::TextColored(ImColor(1.0f, 0.0f, 0.2f, 1.0f), new_list_error);
			}
			ImGui::EndGroup();
			modal_button_group_width = ImGui::GetItemRectSize().x;

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}

	bool texture_dialog::gui()
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

		if (this->rtt_is_focus_pending() || this->is_bring_to_front_pending())
		{
			this->rtt_set_focus_state(false);
			this->set_bring_to_front(false);
			ImGui::SetNextWindowFocus();
		}

		if (!ImGui::Begin("Textures##rtt", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::PopStyleColor(p_colors);
			ImGui::PopStyleVar(p_styles);
			ImGui::End();
			return false;
		}

		if (this->rtt_get_texture())
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

			const float toolbar_height = 32.0f;
			const float frame_height = toolbar_height + (tabbar_visible ? ImGui::GetFrameHeightWithSpacing() : 0.0f);

			const auto& io = ImGui::GetIO();
			this->rtt_set_size(ImGui::GetWindowSize() - ImVec2(12.0f, frame_height));

			const auto pre_child_pos = ImGui::GetCursorScreenPos();

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child", ImVec2(texture_size.x, texture_size.y + frame_height), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				const float pre_toolbar_cursor_y = ImGui::GetCursorPosY();

				{
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6.0f, ImGui::GetCursorPosY() + 6.0f));
					texture_dialog::toolbar();
				}

				ImGui::SetCursorPosY(pre_toolbar_cursor_y + toolbar_height);

				const auto screenpos = ImGui::GetCursorScreenPos();
				const auto& scene_size = this->rtt_get_size();

				ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 8.0f);

				SetWindowPos(
					cmainframe::activewnd->m_pTexWnd->GetWindow(), 
					HWND_BOTTOM, 
					static_cast<int>(screenpos.x),
					static_cast<int>(screenpos.y),
					static_cast<int>(scene_size.x), 
					static_cast<int>(scene_size.y), 
					SWP_NOZORDER);

				const auto pre_image_cursor = ImGui::GetCursorPos();

				ImGui::Image(this->rtt_get_texture(), texture_size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
				this->rtt_set_hovered_state(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup));

				if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
				{
					this->rtt_set_hovered_state(true);
				}
				else
				{
					this->rtt_set_hovered_state(false);
				}

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

				this->rtt_set_cursor_pos(ImVec2(io.MousePos.x - cursor_screen_pos.x, io.MousePos.y - cursor_screen_pos.y));

				// right click context menu
				texture_dialog::context_menu();

				texture_dialog::popup_new_favourite_list();

				// reset cursorpos and fix tabbar triangle
				ImGui::SetCursorScreenPos(pre_child_pos);
				ggui::redraw_undocking_triangle(wnd, this->rtt_get_hovered_state());

				ImGui::EndChild();
			}

			// *
			// custom scrollbar
			if (dvars::gui_texwnd_draw_scrollbar && dvars::gui_texwnd_draw_scrollbar->current.enabled)
			{
				const ImGuiContext& g = *GImGui;
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

				ImS64 scroll = g_texwnd->nPos[0].nPos_current;
				const float size_avail = window->InnerRect.Max[ImGuiAxis_Y] - window->InnerRect.Min[ImGuiAxis_Y];
				const float size_contents = (float)g_texwnd->nPos[0].nPos_max + 110.0f;

				ImGui::ScrollbarEx(bb, id, ImGuiAxis_Y, &scroll, static_cast<ImS64>(size_avail), static_cast<ImS64>(size_contents), rounding_corners);
				//ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

				g_texwnd->nPos[0].nPos_current = static_cast<int>(scroll);
			}
		}

		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();

		return true;
	}

	REGISTER_GUI(texture_dialog);
}
