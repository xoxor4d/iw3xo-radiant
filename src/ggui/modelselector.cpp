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

	bool modelselector_dialog::gui()
	{
		const auto io = ImGui::GetIO();

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		if (!ImGui::Begin("Model Browser##rtt", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

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

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
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

			auto post_button_pos = ImGui::GetCursorPos();

			if (ImGui::ButtonEx("#", ImVec2(0, 0), ImGuiButtonFlags_AllowItemOverlap))
			{
				m_preferences_open = !m_preferences_open;
			} TT("open modelselector settings");

			post_button_pos.x += ImGui::GetItemRectSize().x + 10.0f;

			{
				const auto prefs = game::g_PrefsDlg();
				auto bg_color = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);
				auto bg_color_hovered = ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector);
				auto bg_color_active = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);


				static bool hov_plantmodel;
				if (ggui::toolbar_dialog::image_togglebutton(
					"plant_models",
					hov_plantmodel,
					prefs->m_bDropModel,
					"Plant models and apply random scale and rotation",
					&bg_color,
					&bg_color_hovered,
					&bg_color_active))
				{
					// CMainFrame::OnPlantModel
					mainframe_thiscall(LRESULT, 0x42A0E0);
				}


				static bool hov_plantorient;
				if (ggui::toolbar_dialog::image_togglebutton(
					"plant_orient_to_floor",
					hov_plantorient,
					prefs->m_bOrientModel,
					"Orient dropped selection to the floor",
					&bg_color,
					&bg_color_hovered,
					&bg_color_active))
				{
					// CMainFrame::OnOrientToFloor
					mainframe_thiscall(LRESULT, 0x4258F0);
				}

				static bool hov_plantdrop;
				if (ggui::toolbar_dialog::image_togglebutton(
					"plant_force_drop_height",
					hov_plantdrop,
					prefs->m_bForceZeroDropHeight,
					"Force drop height to 0",
					&bg_color,
					&bg_color_hovered,
					&bg_color_active))
				{
					// CMainFrame::OnForceZeroDropHeight
					mainframe_thiscall(LRESULT, 0x42A000);
				}
			}

			if (m_preferences_open)
			{
				static float buttons_total_width = 448.0f;
				ImGui::SetCursorPos(post_button_pos);

				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
				ImGui::SetNextWindowBgAlpha(0.75f);
				ImGui::BeginChild("settings_child", ImVec2(buttons_total_width + 12.0f, 64.0f), true);
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

						ImGui::SameLine();
						if (ImGui::Button("Reload XModel"))
						{
							const char* model_str = m_preview_model_name.c_str();
							auto hash = game::FS_HashFileName(model_str, 1024);

							// part of Hunk_FindDataForFileInternal
							game::fileData_s* searchFileData = game::com_fileDataHashTable[hash];
							if (searchFileData)
							{
								bool end_of_list = false;
								while (static_cast<std::uint8_t>(searchFileData->type) != 5 || game::I_stricmp(searchFileData->name, model_str))
								{
									searchFileData = searchFileData->next;
									if (!searchFileData)
									{
										end_of_list = true;
										break;
									}
								}

								if(!end_of_list)
								{
									auto hash_entry = &game::com_fileDataHashTable[hash];
									memset(hash_entry, 0, sizeof(uintptr_t));
								}
							}
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

			if(m_preview_model_ptr)
			{
				const auto padding_right = 4.0f;
				const auto upper_right = ImVec2(screenpos_pre_scene.x + this->rtt_get_size().x, screenpos_pre_scene.y);
				ImGui::SetCursorScreenPos(upper_right);

				int vertcount_total = 0;
				const bool calc_total_verts = m_preview_model_ptr->numsurfs > 1;

				{
					const char* bones_str = utils::va("Num Bones: %d", m_preview_model_ptr->numBones);
					ImGui::SetCursorPosX(ImGui::GetCursorPos().x - ImGui::CalcTextSize(bones_str).x - padding_right);
					ImGui::TextUnformatted(bones_str);
					ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y + 2.0f));

					const auto line_y = ImGui::GetCursorScreenPos().y - 2.0f;
					const auto p1 = ImVec2(upper_right.x - ImGui::CalcTextSize(bones_str).x - padding_right, line_y);
					const auto p2 = ImVec2(upper_right.x - padding_right, line_y);
					ImGui::GetWindowDrawList()->AddLine(p1, p2, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
					ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y + 2.0f));
				}

				{
					float lod_str_width = 0.0f;

					if(m_preview_model_ptr->numLods)
					{
						for (auto lod = 0; lod < m_preview_model_ptr->numLods; lod++)
						{
							const char* lod_dist_surfs_str = utils::va("Dist %.1f - Surfs %d [LOD %d]", m_preview_model_ptr->lodInfo[lod].dist, m_preview_model_ptr->lodInfo[lod].numsurfs, lod);
							lod_str_width = ImGui::CalcTextSize(lod_dist_surfs_str).x;

							ImGui::SetCursorPosX(ImGui::GetCursorPos().x - lod_str_width - padding_right);
							ImGui::TextUnformatted(lod_dist_surfs_str);
							ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y));
						}

						ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y + 2.0f));

						const auto line_y = ImGui::GetCursorScreenPos().y;
						const auto p1 = ImVec2(upper_right.x - lod_str_width - padding_right, line_y);
						const auto p2 = ImVec2(upper_right.x - padding_right, line_y);
						ImGui::GetWindowDrawList()->AddLine(p1, p2, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
						ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y + 2.0f));
					}
				}

				{
					const char* vert_str = "Surface Vertcounts";
					ImGui::SetCursorPosX(ImGui::GetCursorPos().x - ImGui::CalcTextSize(vert_str).x - padding_right);
					ImGui::TextUnformatted(vert_str);
					ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y));

					for (auto surf = 0; surf < m_preview_model_ptr->numsurfs; surf++)
					{
						vertcount_total += m_preview_model_ptr->surfs[surf].vertCount;

						const char* surf_vert_count_str = calc_total_verts ?
							utils::va("%d [%d]", m_preview_model_ptr->surfs[surf].vertCount, surf)
							: utils::va("%d [=]", m_preview_model_ptr->surfs[surf].vertCount);

						ImGui::SetCursorPosX(ImGui::GetCursorPos().x - ImGui::CalcTextSize(surf_vert_count_str).x - padding_right);
						ImGui::TextUnformatted(surf_vert_count_str);
						ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y));
					}
				}

				if(calc_total_verts)
				{
					const char* vert_total_str = utils::va("%d [=]", vertcount_total);

					const auto line_y = ImGui::GetCursorScreenPos().y - 1.0f;
					const auto p1 = ImVec2(upper_right.x - ImGui::CalcTextSize(vert_total_str).x - padding_right, line_y);
					const auto p2 = ImVec2(upper_right.x - padding_right, line_y);
					ImGui::GetWindowDrawList()->AddLine(p1, p2, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);


					ImGui::SetCursorPosX(ImGui::GetCursorPos().x - ImGui::CalcTextSize(vert_total_str).x - padding_right);
					ImGui::TextUnformatted(vert_total_str);
					ImGui::SetCursorScreenPos(ImVec2(upper_right.x, ImGui::GetCursorScreenPos().y));
				}
				

			}
		}

		ImGui::EndChild();
		ImGui::End();
		return true;
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
		components::command::register_command_with_hotkey("xo_modelselector"s, [this](auto)
		{
			if (this->is_inactive_tab() && this->is_active())
			{
				this->set_bring_to_front(true);
				return;
			}

			this->toggle();
		});
	}

	REGISTER_GUI(modelselector_dialog);
}
