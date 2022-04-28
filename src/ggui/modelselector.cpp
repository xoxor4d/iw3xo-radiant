#include "std_include.hpp"

namespace ggui
{
	void modelselector_dialog::xmodel_listbox_elem(int index)
	{
		//const auto m_selector = ggui::get_rtt_modelselector();

		const bool is_selected = (this->m_xmodel_selection == index);
		if (ImGui::Selectable(this->m_xmodel_filelist[index], is_selected))
		{
			this->m_xmodel_selection = index;
			this->m_preview_model_name = this->m_xmodel_filelist[index];
		} TT("Drag me to the grid or camera window");

		if (is_selected) {
			ImGui::SetItemDefaultFocus();
		}

		if (is_selected && m_update_scroll_position)
		{
			ImGui::SetScrollHereY();
			m_update_scroll_position = false;
		}

		// target => cxywnd::rtt_grid_window()
		// target => ccamwnd::rtt_camera_window()
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("MODEL_SELECTOR_ITEM", nullptr, 0, ImGuiCond_Once);

			this->m_xmodel_selection = index;
			this->m_preview_model_name = this->m_xmodel_filelist[index];

			ImGui::Text("Model: %s", this->m_xmodel_filelist[index]);
			ImGui::EndDragDropSource();
		}
	}

	void modelselector_dialog::gui()
	{
		//const auto m_selector = ggui::get_rtt_modelselector();
		const auto io = ImGui::GetIO();

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		if (this->is_bring_to_front_pending())
		{
			this->set_bring_to_front(false);
			ImGui::SetNextWindowFocus();
		}

		if (!ImGui::Begin("Model Selector / Previewer", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			this->set_inactive_tab(true);
			ImGui::End();
			return;
		}

		this->set_inactive_tab(false);

		// block all hotkeys if window is focused (cmainframe::on_keydown())
		this->rtt_set_hovered_state(ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows));

		const auto window_size = ImGui::GetWindowSize();
		const bool split_horizontal = window_size.x >= 700.0f;
		const auto listbox_width = split_horizontal ? ImClamp(window_size.x * 0.25f, 200.0f, FLT_MAX) : ImGui::GetContentRegionAvail().x;

		ImGui::BeginGroup();
		{
			// filter widget
			const auto screenpos_prefilter = ImGui::GetCursorScreenPos();
			this->m_filter.Draw("##xmodel_filter", listbox_width - 32.0f);
			const auto screenpos_postfilter = ImGui::GetCursorScreenPos();

			ImGui::SameLine(listbox_width - 27.0f);
			if (ImGui::ButtonEx("x##clear_filter"))
			{
				this->m_filter.Clear();
			}

			if (!this->m_filter.IsActive())
			{
				ImGui::SetCursorScreenPos(ImVec2(screenpos_prefilter.x + 12.0f, screenpos_prefilter.y + 4.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
				ImGui::TextUnformatted("Filter ..");
				ImGui::PopStyleColor();
				ImGui::SetCursorScreenPos(ImVec2(screenpos_postfilter.x, screenpos_postfilter.y));
			}

			// xmodel listbox
			if (ImGui::BeginListBox("##model_list", ImVec2(listbox_width, split_horizontal ? ImGui::GetContentRegionAvail().y : ImGui::GetContentRegionAvail().y * 0.5f)))
			{
				if (this->m_filter.IsActive())
				{
					for (int i = 0; i < this->m_xmodel_filecount; i++)
					{
						if (!this->m_filter.PassFilter(this->m_xmodel_filelist[i]))
						{
							continue;
						}

						xmodel_listbox_elem(i);
					}
				}
				else
				{
					ImGuiListClipper clipper;
					clipper.Begin(this->m_xmodel_filecount);

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

		if (this->rtt_is_hovered())
		{
			if (ImGui::IsKeyPressedMap(ImGuiKey_DownArrow))
			{
				if (this->m_xmodel_selection + 1 < this->m_xmodel_filecount)
				{
					if (this->m_filter.IsActive())
					{
						for (int i = this->m_xmodel_selection + 1; i < this->m_xmodel_filecount; i++)
						{
							if (!this->m_filter.PassFilter(this->m_xmodel_filelist[i]))
							{
								continue;
							}

							this->m_xmodel_selection = i;
							this->m_preview_model_name = this->m_xmodel_filelist[this->m_xmodel_selection];

							m_update_scroll_position = true;
							break;
						}
					}
					else
					{
						this->m_xmodel_selection += 1;
						this->m_preview_model_name = this->m_xmodel_filelist[this->m_xmodel_selection];

						m_update_scroll_position = true;
					}
				}
			}
			else if (ImGui::IsKeyPressedMap(ImGuiKey_UpArrow))
			{
				if (this->m_xmodel_selection - 1 >= 0)
				{
					if (this->m_filter.IsActive())
					{
						for (int i = this->m_xmodel_selection - 1; i >= 0; i--)
						{
							if (!this->m_filter.PassFilter(this->m_xmodel_filelist[i])) {
								continue;
							}

							this->m_xmodel_selection = i;
							this->m_preview_model_name = this->m_xmodel_filelist[this->m_xmodel_selection];

							m_update_scroll_position = true;
							break;
						}
					}
					else
					{
						this->m_xmodel_selection -= 1;
						this->m_preview_model_name = this->m_xmodel_filelist[this->m_xmodel_selection];

						m_update_scroll_position = true;
					}
				}
			}
		}

		if (split_horizontal)
		{
			ImGui::SameLine();
		}

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
		{
			static bool was_mouse_hidden = false;

			const auto child_size = ImGui::GetContentRegionAvail();
			SetWindowPos(layermatwnd_struct->m_content_hwnd, HWND_BOTTOM, 0, 0, (int)child_size.x, (int)child_size.y, SWP_NOZORDER);
			this->rtt_set_size(child_size);

			const auto screenpos_pre_scene = ImGui::GetCursorScreenPos();

			if (this->rtt_get_texture())
			{
				ImGui::Image(this->rtt_get_texture(), child_size);
				this->m_scene_texture_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				// target => cxywnd::rtt_grid_window()
				// target => ccamwnd::rtt_camera_window()
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					// tell czwnd::on_lbutton_up() to pass left mouse input to imgui (on release, once)
					ggui::dragdrop_overwrite_leftmouse_capture();

					ImGui::SetDragDropPayload("MODEL_SELECTOR_ITEM", nullptr, 0, ImGuiCond_Once);
					ImGui::Text("Model: %s", this->m_preview_model_name.c_str());
					ImGui::EndDragDropSource();
				}

				if (this->m_scene_texture_hovered)
				{
					// zoom
					if (io.MouseWheel != 0.0f)
					{
						this->m_camera_distance -= (io.MouseWheel * 10.0f);
					}

					if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
					{
						CPoint point;
						GetCursorPos(&point);

						if (!ImGui::IsMouseDragging(ImGuiMouseButton_Right))
						{
							this->rtt_set_cursor_pos_cpoint(point);
						}
						else
						{
							if (point.x != this->rtt_get_cursor_pos_cpoint().x || point.y != this->rtt_get_cursor_pos_cpoint().y)
							{
								const auto angle_speed = static_cast<float>(game::g_PrefsDlg()->m_nAngleSpeed);

								// rotation mod
								if (io.KeyMods == (ImGuiKeyModFlags_Ctrl | ImGuiKeyModFlags_Shift))
								{
									this->m_camera_angles[2] = this->m_camera_angles[2] - angle_speed / 500.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y);
								}
								// translation
								else if (io.KeyMods == ImGuiKeyModFlags_Ctrl)
								{
									const auto move_speed = static_cast<float>(game::g_PrefsDlg()->m_nMoveSpeed);
									this->m_camera_offset[1] = (this->m_camera_offset[1] + move_speed / 1000.0f * static_cast<float>(point.x - this->rtt_get_cursor_pos_cpoint().x));
									this->m_camera_offset[2] = (this->m_camera_offset[2] - move_speed / 1000.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y));
								}
								// normal rotation
								else
								{
									this->m_user_rotation = true;
									this->m_camera_angles[1] = this->m_camera_angles[1] - angle_speed / 500.0f * static_cast<float>(point.x - this->rtt_get_cursor_pos_cpoint().x);
									this->m_camera_angles[0] = this->m_camera_angles[0] - angle_speed / 500.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y);
								}

								SetCursorPos(this->rtt_get_cursor_pos_cpoint().x, this->rtt_get_cursor_pos_cpoint().y);
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
					this->m_user_rotation = false;
				}
			}

			/*  + render methods +
			 *	fullbright		= 4,
			 *	normal-based	= 24,
			 *	view-based		= 25,
			 *	case-textures	= 27,
			 *	wireframe white = 28,
			 *	wireframe		= 29,
			 */

			ImGui::SetItemAllowOverlap();
			ImGui::SetCursorScreenPos(ImVec2(screenpos_pre_scene.x, screenpos_pre_scene.y));

			if (ImGui::ButtonEx("#", ImVec2(0, 0), ImGuiButtonFlags_AllowItemOverlap))
			{
				m_preferences_open = !m_preferences_open;
			} TT("open modelselector settings");

			if (m_preferences_open)
			{
				static float buttons_total_width = 448.0f;

				ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				ImGui::SetNextWindowBgAlpha(0.75f);
				ImGui::BeginChild("settings_child", ImVec2(buttons_total_width + 12.0f, 64.0f), true, ImGuiWindowFlags_None);
				{
					ImGui::BeginGroup();
					{
						ImGui::PushStyleCompact();
						if (ImGui::Button("Fullbright")) {
							layermatwnd::rendermethod_preview = layermatwnd::FULLBRIGHT;
						}

						ImGui::SameLine();
						if (ImGui::Button("Normal")) {
							layermatwnd::rendermethod_preview = layermatwnd::FAKELIGHT_NORMAL;
						}

						ImGui::SameLine();
						if (ImGui::Button("View")) {
							layermatwnd::rendermethod_preview = layermatwnd::FAKELIGHT_VIEW;
						}

						ImGui::SameLine();
						if (ImGui::Button("Case")) {
							layermatwnd::rendermethod_preview = layermatwnd::CASE_TEXTURE;
						}

						ImGui::SameLine();
						if (ImGui::Button("Sun")) {
							layermatwnd::rendermethod_preview = layermatwnd::FAKESUN_DAY;
						}

						ImGui::SameLine();
						if (ImGui::Button("Wireframe"))
						{
							layermatwnd::rendermethod_preview = layermatwnd::rendermethod_preview != layermatwnd::WIREFRAME_WHITE ? layermatwnd::WIREFRAME_WHITE : layermatwnd::WIREFRAME;
						} TT("Press a second time to switch to textured wireframe");

						ImGui::SameLine();
						if (ImGui::Button(this->m_anim_pause ? "Play" : "Pause", ImVec2(56.0f, 0)))
						{
							this->m_anim_pause = !this->m_anim_pause;
						} TT("play/pause automatic rotation");

						ImGui::PopStyleCompact();
					}
					ImGui::EndGroup();

					buttons_total_width = ImGui::GetItemRectSize().x;

					ImGui::PushStyleCompact();

					ImGui::BeginGroup();
					{
						if (ImGui::Button(this->m_overwrite_selection ? "Overwrite selection: On" : "Overwrite selection: Off", ImVec2(168.0f, 0))) {
							this->m_overwrite_selection = !this->m_overwrite_selection;
						} TT("On: swap model for selected entity\nOff: spawn a new entity");

						ImGui::SameLine();
						if (ImGui::Button("Refresh List"))
						{
							this->set_initiated(true);
							this->m_xmodel_selection = -1;
						}

						ImGui::EndGroup();
					}

					const float second_row_button_width = ImGui::GetItemRectSize().x;

					ImGui::SetNextItemWidth(buttons_total_width - ImGui::CalcTextSize("Fov").x - 24.0f - second_row_button_width);
					ImGui::SameLine();
					ImGui::DragFloat("Fov", &this->m_camera_fov, 0.1f, 10.0f, 125.0f);
					ImGui::PopStyleCompact();
				}
				ImGui::EndChild();
				ImGui::PopStyleVar();
			}

			{
				static bool hov_plantorient;
				const auto prefs = game::g_PrefsDlg();

				auto bg_color = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);
				auto bg_color_hovered = ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector);
				auto bg_color_active = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);

				// CMainFrame::OnPlantModel
				if (ggui::toolbar_dialog::image_togglebutton(
					"plant_orient_to_floor",
					hov_plantorient,
					prefs->m_bOrientModel,
					"Orient dropped selection to the floor",
					&bg_color,
					&bg_color_hovered,
					&bg_color_active))
				{
					mainframe_thiscall(LRESULT, 0x4258F0);
				}

				static bool hov_plantdrop;

				// CMainFrame::OnForceZeroDropHeight
				if (ggui::toolbar_dialog::image_togglebutton(
					"plant_force_drop_height",
					hov_plantdrop,
					prefs->m_bForceZeroDropHeight,
					"Force drop height to 0",
					&bg_color,
					&bg_color_hovered,
					&bg_color_active))
				{
					mainframe_thiscall(LRESULT, 0x42A000);
				}
			}
			

			const auto current_pos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(split_horizontal ? current_pos.x : current_pos.x + 4.0f, ImGui::GetWindowHeight() - 22.0f));
			ImGui::HelpMarker(
				"Rotate Object:\t\tRight Mouse\n"
				"Rotate Modifier:\t Right Mouse +Ctrl\n"
				"Move Object:\t\t  Right Mouse +Ctrl +Shift\n"
				"Zoom Object:\t\t  Mouse Scroll\n"
				"Change selection:  Up/Down Arrow:\n"
				"- Drag and drop a xmodel from the list/preview to the grid or camera to spawn it\n"
				" ^ use model planting settings (toolbar) like 'Orient To Floor' or 'Force Zero Dropheight'\n"
				" ^ to adjust model planing behaviour");

			if (this->m_bad_model)
			{
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Bad model");
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}

	void modelselector_dialog::on_open()
	{
		if (!this->is_initiated())
		{
			this->m_xmodel_filelist = game::FS_ListFilteredFilesWrapper("xmodel", nullptr, &this->m_xmodel_filecount);
			this->set_initiated();
		}
	}

	void modelselector_dialog::on_close()
	{ }

	void modelselector_dialog::init()
	{
		components::command::register_command_with_hotkey("xo_modelselector"s, [](auto)
		{
			const auto gui = GET_GUI(ggui::modelselector_dialog); //ggui::get_rtt_modelselector();

			if (gui->is_inactive_tab() && gui->is_active())
			{
				gui->set_bring_to_front(true);
				return;
			}

			gui->toggle();
		});
	}

	REGISTER_GUI(modelselector_dialog);
}
