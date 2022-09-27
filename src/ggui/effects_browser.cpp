#include "std_include.hpp"

namespace ggui
{
	enum FX_ELEM_RESULT_
	{
		FX_ELEM_RESULT_NONE,
		FX_ELEM_RESULT_SELECTED,
		FX_ELEM_RESULT_DRAGGED
	};

	int effects_browser::image_button(const game::GfxImage* img, const float img_size, const char* label, const char* img_ico_awesomefont, ImVec2 uv0, ImVec2 uv1)
	{
		int result = FX_ELEM_RESULT_NONE;
		int styles = 0;

		imgui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); styles++;

		if (img)
		{
			imgui::Image(img->texture.data, ImVec2(img_size, img_size), uv0, uv1);
		}
		else
		{
			imgui::TextUnformatted(img_ico_awesomefont);
		}

		imgui::PopStyleVar(styles);

		imgui::SameLine();

		//imgui::PushStyleColor(ImGuiCol_Header, ImVec4(1.00f, 0.12f, 0.12f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.23f, 0.23f, 0.23f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

		if (imgui::Selectable(utils::va(label), false, ImGuiSelectableFlags_SpanAvailWidth | ImGuiSelectableFlags_SetNavIdOnHover | ImGuiSelectableFlags_DrawHoveredWhenHeld))
		{
			result = FX_ELEM_RESULT_SELECTED;
		}

		// hack to detect dragging
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::EndDragDropSource();
			result = FX_ELEM_RESULT_DRAGGED;
		}

		imgui::PopStyleColor(2);

		return result;
	}

	void effects_browser::handle_drag_drop()
	{
		if (!m_dragdrop_fx_name.empty())
		{
			// target => grid_dialog::drag_drop_target()
			// target => camera_dialog::drag_drop_target()
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				const char* prefab_name = m_dragdrop_fx_name.c_str();
				const char* prefab_path = m_dragdrop_fx_path.c_str();

				const bool is_root = !prefab_path || prefab_path[0] == '\0';
				const char* full_path = utils::va(is_root ? "%s%s" : "%s\\%s", is_root ? "" : prefab_path, prefab_name);

				ImGui::SetDragDropPayload("EFFECT_BROWSER_ITEM", full_path, strlen(full_path), ImGuiCond_Once);
				ImGui::Text("Effect: %s", full_path);
				ImGui::EndDragDropSource();
			}
		}
	}

	void effects_browser::update_directory()
	{
		m_curr_dir_folders.clear();
		m_curr_dir_files.clear();

		for (auto& it : std::filesystem::directory_iterator(m_current_directory))
		{
			const auto& full_path = it.path();
			const auto rel_path = std::filesystem::relative(full_path, m_fx_directory);

			if (it.is_directory())
			{
				m_curr_dir_folders.emplace_back(rel_path.filename().string());
			}
			else
			{
				if (full_path.extension() == ".efx")
				{
					std::string file_str = rel_path.filename().string();
					utils::erase_substring(file_str, ".efx");

					m_curr_dir_files.emplace(
						std::make_pair(file_str, rel_path.has_parent_path() ? rel_path.parent_path() : ""));
				}
			}
		}
	}

	bool effects_browser::gui()
	{
		const auto io = ImGui::GetIO();

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		if (!ImGui::Begin("Effects Browser##rtt", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		const auto window_size = ImGui::GetWindowSize();
		const bool split_horizontal = window_size.x >= 700.0f;
		const auto listbox_width = split_horizontal ? ImClamp(window_size.x * 0.25f, 200.0f, FLT_MAX) : ImGui::GetContentRegionAvail().x;

		ImGui::BeginGroup();
		{
			imgui::BeginDisabled(m_current_directory == m_fx_directory);
			{
				if (imgui::Button("<"))
				{
					m_current_directory = m_current_directory.parent_path();
					update_directory();
				}

				imgui::EndDisabled();
			}

			imgui::SameLine();

			imgui::BeginGroup();
			{
				// filter widget
				const auto screenpos_prefilter = ImGui::GetCursorScreenPos();
				this->m_filter.Draw("##xmodel_filter", listbox_width - 70.0f);

				ImGui::SameLine(listbox_width - 64.0f);
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
				}

				imgui::EndGroup();
			}

			imgui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
			ImGui::BeginChild("##effect_list", ImVec2(listbox_width, split_horizontal ? ImGui::GetContentRegionAvail().y : ImGui::GetContentRegionAvail().y * 0.5f), true);
			{
				if (const auto rel_path = std::filesystem::relative(m_current_directory, m_fx_directory);
					!rel_path.empty() && rel_path != ".")
				{
					imgui::TextUnformatted((" > \\" + rel_path.string()).c_str());
				}
				else
				{
					imgui::TextUnformatted(" > \\");
				}

				game::GfxImage* folder_ico;
				if (folder_ico = game::Image_RegisterHandle("folder"); !folder_ico)
				{
					folder_ico = game::Image_RegisterHandle("invalid_texture");
				}

				int folder_num = 0;
				for (const auto& folder : m_curr_dir_folders)
				{
					bool updated = false;

					imgui::PushID(folder_num);
					imgui::PushFontFromIndex(E_FONT::BOLD_18PX);


					if (effects_browser::image_button(folder_ico, 18.0f, folder.c_str()))
					{
						m_current_directory /= folder;
						update_directory();
						updated = true;
					}

					imgui::PopFont();
					imgui::PopID();
					folder_num++;

					if (updated)
					{
						break;
					}
				}

				for (const auto& [filename, parent_path] : m_curr_dir_files)
				{
					if (this->m_filter.IsActive() && !this->m_filter.PassFilter(filename.c_str()))
					{
						continue;
					}

					//imgui::TableNextColumn();


					bool update_drag_drop = false;
					const char* filename_str = filename.c_str();

					
					imgui::PushID(filename_str);
					if (const auto ret = effects_browser::image_button(nullptr, 18.0f, filename_str); 
						ret != FX_ELEM_RESULT_NONE)
					{
						if (ret == FX_ELEM_RESULT_SELECTED)
						{
							this->m_effect_selection = parent_path.string() + "\\" + filename;

							if (m_effect_selection != m_effect_selection_old)
							{
								this->m_effect_selection_old = m_effect_selection;

								cfxwnd::get()->stop_effect();
								cfxwnd::get()->load_effect(this->m_effect_selection.c_str());
							}
							else
							{
								// restart effect

								cfxwnd::get()->retrigger_effect(cfxwnd::get()->m_tickcount_playback);
							}
						}

						const auto current_id = imgui::GetItemID();
						if (!m_dragdrop_id || m_dragdrop_id != current_id)
						{
							m_dragdrop_id = current_id;
							update_drag_drop = true;
						}
					}
					imgui::PopID();

					if (update_drag_drop)
					{
						//game::printf_to_console("updating drag drop");
						m_dragdrop_fx_name = filename;
						m_dragdrop_fx_path = parent_path.string();
					}

					handle_drag_drop();
				}
			} ImGui::EndChild();
			imgui::PopStyleColor();

			//imgui::SetCursorScreenPos(ImVec2(imgui::GetCursorScreenPos().x, cursor_y));

		}
		ImGui::EndGroup();

		if (split_horizontal)
		{
			ImGui::SameLine();
		}


		
		const auto pre_render_pos = imgui::GetCursorScreenPos();

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			static bool was_mouse_hidden = false;

			const auto child_size = ImGui::GetContentRegionAvail();
			SetWindowPos(components::renderer::get_window(components::renderer::CFXWND)->hwnd, HWND_BOTTOM, 0, 0, static_cast<int>(child_size.x), static_cast<int>(child_size.y), SWP_NOZORDER);
			this->rtt_set_size(child_size);

			if (this->rtt_get_texture())
			{
				ImGui::Image(this->rtt_get_texture(), child_size);
				this->m_scene_texture_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				if (this->m_scene_texture_hovered)
				{
					// zoom
					if (io.MouseWheel != 0.0f)
					{
						this->m_camera_distance += (io.MouseWheel * 10.0f);
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
								const auto cfx = cfxwnd::get();

								const auto angle_speed = static_cast<float>(game::g_PrefsDlg()->m_nAngleSpeed);
								const auto move_speed = static_cast<float>(game::g_PrefsDlg()->m_nMoveSpeed);

								// rotation
								if (io.KeyMods == (ImGuiKeyModFlags_Ctrl | ImGuiKeyModFlags_Shift))
								{
									cfx->m_angles[1] = cfx->m_angles[1] - angle_speed / 500.0f * static_cast<float>(point.x - this->rtt_get_cursor_pos_cpoint().x);
									cfx->m_angles[0] = cfx->m_angles[0] - angle_speed / 500.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y);
								}

								// pan
								else if (io.KeyMods == ImGuiKeyModFlags_Ctrl)
								{
									const float x_delta = move_speed / 300.0f * static_cast<float>(point.x - this->rtt_get_cursor_pos_cpoint().x);
									const float y_delta = move_speed / 300.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y);

									cfx->m_origin[0] = x_delta * cfx->m_vright[0] + cfx->m_origin[0];
									cfx->m_origin[1] = x_delta * cfx->m_vright[1] + cfx->m_origin[1];
									cfx->m_origin[2] = x_delta * cfx->m_vright[2] + cfx->m_origin[2];
									cfx->m_origin[2] = cfx->m_origin[2] - y_delta;
								}

								// forward translation and left right rot (radiant cam)
								else
								{
									

									// use anglespeed
									cfx->m_angles[1] = cfx->m_angles[1] - angle_speed / 500.0f * static_cast<float>(point.x - this->rtt_get_cursor_pos_cpoint().x);

									const float delta = move_speed / -250.0f * static_cast<float>(point.y - this->rtt_get_cursor_pos_cpoint().y);

									game::vec3_t vpn = {};
									utils::vector::normalize2(cfx->m_vpn, vpn);

									cfx->m_origin[0] = vpn[0] * delta + cfx->m_origin[0];
									cfx->m_origin[1] = vpn[1] * delta + cfx->m_origin[1];
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
					int  sw_cur;
					do { sw_cur = ShowCursor(1); } while (sw_cur < 0);

					was_mouse_hidden = false;
				}

				handle_drag_drop();
			}
		}

		ImGui::SetItemAllowOverlap();
		imgui::SetCursorScreenPos(pre_render_pos);

		imgui::BeginGroup();
		{
			if (cfxwnd::get()->m_effect_is_using_physx)
			{
				imgui::Text("spawn origin adjusted to [0, 0, 80] because effect is using physics");
				imgui::Text("actors %d", components::physx_impl::get()->m_phys_active_actor_fx_browser_count);
				imgui::Text("tick msec %d", components::physx_impl::get()->m_phys_fx_browser_msec_step);
			}

			//imgui::Text("m_effect_is_playing %s", cfxwnd::get()->m_effect_is_playing ? "true" : "false");
			//imgui::Text("m_tickcount_playback %d", cfxwnd::get()->m_tickcount_playback);
			//imgui::Text("m_tickcount_repeat %d", cfxwnd::get()->m_saved_tick_old);

			imgui::EndGroup();
		}

		ImGui::EndChild();

		ImGui::End();

		return true;
	}

	void effects_browser::on_open()
	{
		const auto egui = GET_GUI(entity_dialog);
		m_fx_directory = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
		m_fx_directory /= "raw\\fx";

		m_current_directory = m_fx_directory;
		update_directory();
	}

	void effects_browser::on_close()
	{ }

	REGISTER_GUI(effects_browser);
}
