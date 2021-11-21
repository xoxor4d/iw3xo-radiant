#include "std_include.hpp"

namespace ggui::modelselector
{
	ImGuiTextFilter	m_filter;
	bool update_scroll_pos = false;

	void xmodel_listbox_elem(int index)
	{
		const auto m_selector = ggui::get_rtt_modelselector();
		
		const bool is_selected = (m_selector->xmodel_selection == index);
		if (ImGui::Selectable(m_selector->xmodel_filelist[index], is_selected))
		{
			m_selector->xmodel_selection = index;
			m_selector->preview_model_name = m_selector->xmodel_filelist[index];
		} TT("Drag me to the grid or camera window");

		if (is_selected) {
			ImGui::SetItemDefaultFocus();
		}

		if (is_selected && update_scroll_pos)
		{
			ImGui::SetScrollHereY();
			update_scroll_pos = false;
		}

		// target => cxywnd::rtt_grid_window()
		// target => ccamwnd::rtt_camera_window()
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("MODEL_SELECTOR_ITEM", nullptr, 0, ImGuiCond_Once);

			m_selector->xmodel_selection = index;
			m_selector->preview_model_name = m_selector->xmodel_filelist[index];

			ImGui::Text("Model: %s", m_selector->xmodel_filelist[index]);
			ImGui::EndDragDropSource();
		}
	}
	
	void menu()
	{
		const auto m_selector = ggui::get_rtt_modelselector();
		const auto io = ImGui::GetIO();
		
		if (!m_selector->one_time_init)
		{
			m_selector->xmodel_filelist = game::FS_ListFilteredFilesWrapper("xmodel", nullptr, &m_selector->xmodel_filecount);
			m_selector->one_time_init = true;
		}

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		ImGui::Begin("Model Selector / Previewer", &m_selector->menustate, ImGuiWindowFlags_NoCollapse);

		//if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) __debugbreak();
		// block all hotkeys if window is focused (cmainframe::on_keydown())
		m_selector->window_hovered = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
		
		const auto window_size = ImGui::GetWindowSize();
		const auto listbox_width = ImClamp(window_size.x * 0.25f, 200.0f, FLT_MAX);

		ImGui::BeginGroup();
		{
			// filter widget
			const auto screenpos_prefilter = ImGui::GetCursorScreenPos();
			m_filter.Draw("##xmodel_filter", listbox_width - 26);
			const auto screenpos_postfilter = ImGui::GetCursorScreenPos();

			ImGui::SameLine(listbox_width - 22);
			if (ImGui::ButtonEx("x##clear_filter"))
			{
				m_filter.Clear();
			}

			if (!m_filter.IsActive())
			{
				ImGui::SetCursorScreenPos(ImVec2(screenpos_prefilter.x + 12.0f, screenpos_prefilter.y + 4.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
				ImGui::TextUnformatted("Filter ..");
				ImGui::PopStyleColor();
				ImGui::SetCursorScreenPos(ImVec2(screenpos_postfilter.x, screenpos_postfilter.y));
			}

			// xmodel listbox
			if (ImGui::BeginListBox("##model_list", ImVec2(listbox_width, ImGui::GetContentRegionAvail().y)))
			{
				if (m_filter.IsActive())
				{
					for (int i = 0; i < m_selector->xmodel_filecount; i++)
					{
						if (!m_filter.PassFilter(m_selector->xmodel_filelist[i])) {
							continue;
						}

						xmodel_listbox_elem(i);
					}
				}
				else
				{
					ImGuiListClipper clipper;
					clipper.Begin(m_selector->xmodel_filecount);

					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							xmodel_listbox_elem(i);
						}
					}
					clipper.End();
				}

				ImGui::EndListBox();
			}
		}
		ImGui::EndGroup();

		if(ImGui::IsKeyPressedMap(ImGuiKey_DownArrow))
		{
			if(m_selector->xmodel_selection + 1 < m_selector->xmodel_filecount)
			{
				m_selector->xmodel_selection += 1;
				m_selector->preview_model_name = m_selector->xmodel_filelist[m_selector->xmodel_selection];

				update_scroll_pos = true;
			}
		}
		else if(ImGui::IsKeyPressedMap(ImGuiKey_UpArrow))
		{
			if (m_selector->xmodel_selection - 1 >= 0)
			{
				m_selector->xmodel_selection -= 1;
				m_selector->preview_model_name = m_selector->xmodel_filelist[m_selector->xmodel_selection];

				update_scroll_pos = true;
			}
		}
		
		ImGui::SameLine();

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
		{
			static bool was_mouse_hidden = false;
			
			const auto child_size = ImGui::GetContentRegionAvail();
			SetWindowPos(layermatwnd_struct->m_content_hwnd, HWND_BOTTOM, 0, 0, (int)child_size.x, (int)child_size.y, SWP_NOZORDER);
			m_selector->scene_size_imgui = child_size;

			const auto screenpos_pre_scene = ImGui::GetCursorScreenPos();
			
			if (m_selector->scene_texture)
			{
				ImGui::Image(m_selector->scene_texture, child_size);
				m_selector->scene_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				if (m_selector->scene_hovered)
				{
					// zoom
					if (io.MouseWheel != 0.0f)
					{
						m_selector->camera_distance -= (io.MouseWheel * 10.0f);
					}

					// rotation
					if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
					{
						CPoint point;
						GetCursorPos(&point);
						
						if(!ImGui::IsMouseDragging(ImGuiMouseButton_Right))
						{
							m_selector->m_ptCursor = point;
						}
						else
						{
							if (point.x != m_selector->m_ptCursor.x || point.y != m_selector->m_ptCursor.y)
							{
								const float angle_speed = static_cast<float>(game::g_PrefsDlg()->m_nAngleSpeed);

								if (io.KeyMods == ImGuiKeyModFlags_Ctrl)
								{
									//m_selector->camera_offset[2] = m_selector->camera_offset[2] - angle_speed / 500.0f * static_cast<float>(point.x - m_selector->m_ptCursor.x);
									//m_selector->camera_offset[1] = m_selector->camera_offset[1] - angle_speed / 500.0f * static_cast<float>(point.y - m_selector->m_ptCursor.y);
									m_selector->camera_angles[2] = m_selector->camera_angles[2] - angle_speed / 500.0f * static_cast<float>(point.y - m_selector->m_ptCursor.y);
								}
								else
								{
									m_selector->user_rotation = true;
									m_selector->camera_angles[1] = m_selector->camera_angles[1] - angle_speed / 500.0f * static_cast<float>(point.x - m_selector->m_ptCursor.x);
									m_selector->camera_angles[0] = m_selector->camera_angles[0] - angle_speed / 500.0f * static_cast<float>(point.y - m_selector->m_ptCursor.y);
								}
								
								SetCursorPos(m_selector->m_ptCursor.x, m_selector->m_ptCursor.y);
								ShowCursor(0);
								was_mouse_hidden = true;
							}
						}
					}
				}

				if (!ImGui::IsMouseDown(ImGuiMouseButton_Right) && was_mouse_hidden)
				{
					int cursor_state;
					do {
						cursor_state = ShowCursor(1);
					} while (cursor_state < 0);
					
					was_mouse_hidden = false;
					m_selector->user_rotation = false;
				}
			}

			ImGui::SetCursorScreenPos(ImVec2(screenpos_pre_scene.x + 10, screenpos_pre_scene.y));

			ImGui::HelpMarker(
				"Right Mouse:\t\t\t  Rotate Object\n"
				"Right Mouse + CTRL: Rotate Modifier\n"
				"Mouse Scroll:\t\t\t  Zoom\n"
				"Up/Down Arrow:\t\tChange selection\n"
				"- Drag and drop xmodel from list to grid or camera to spawn it");
		}
		
		ImGui::EndChild();
		ImGui::End();
	}

	void init()
	{
		components::command::register_command_with_hotkey("xo_modelselector"s, [](auto)
		{
			const auto m_selector = ggui::get_rtt_modelselector();
			m_selector->menustate = !m_selector->menustate;
		});
	}
}
