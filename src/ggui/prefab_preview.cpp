#include "std_include.hpp"

namespace ggui
{
	std::filesystem::path prefab_browser_generate_thumbnails_folder;
	bool prefab_browser_generate_thumbnails = false;


	std::string prefab_preview_dialog::get_thumbnail_string(const std::filesystem::path& dir, const std::string& filename)
	{
		std::string full_path = (!dir.empty() ? dir.string() + "\\" : "") + filename;

		std::ranges::transform(full_path.begin(), full_path.end(), full_path.begin(),
			[](unsigned char c) { return std::tolower(c); });

		utils::replace(full_path, "\\", "#");
		utils::replace(full_path, " ", "_");
		utils::replace(full_path, ".map", "");

		return full_path;
	}

	void prefab_preview_dialog::update_directory()
	{
		m_curr_dir_folders.clear();
		m_curr_dir_files.clear();

		for (auto& it : std::filesystem::directory_iterator(m_current_directory))
		{
			const auto& full_path = it.path();
			const auto rel_path = std::filesystem::relative(full_path, m_mapsource_directory);

			if (it.is_directory())
			{
				m_curr_dir_folders.emplace_back(rel_path.filename().string());
			}
			else
			{
				if (full_path.extension() == ".map")
				{
					m_curr_dir_files.emplace(
						std::make_pair(rel_path.filename().string(), rel_path.has_parent_path() ? rel_path.parent_path() : ""));
				}
			}
		}
	}

	bool prefab_preview_dialog::image_button(const game::GfxImage* img, const float size, const float scale, const char* label, bool enable_highlight, int padding, ImVec2 uv0, ImVec2 uv1)
	{
		bool result = false;

		imgui::BeginGroup();
		{
			int styles = 0;

			imgui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); styles++;
			/*const auto cursor = imgui::GetCursorPos();

			imgui::InvisibleButton("##dummy_button", ImVec2(size, size));

			const bool hovered = imgui::IsItemHovered();
			imgui::SetItemAllowOverlap();
			imgui::SetCursorPos(cursor);*/

			if (imgui::ImageButtonScaled(img->texture.data,
				ImVec2(size, size),
				scale,
				uv0,
				uv1,
				padding))
			{
				result = true;
			}
			const bool hovered = imgui::IsItemHovered();

			if (enable_highlight && hovered)
			{
				const auto mins = imgui::GetItemRectMin();
				const auto maxs = imgui::GetItemRectMax();
				imgui::GetWindowDrawList()->AddRectFilled(mins, maxs, imgui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.15f)));
			}


			imgui::PopStyleVar(styles);

			imgui::PushFontFromIndex(REGULAR_14PX);
			imgui::TextUnformatted(label);
			imgui::PopFont();

			imgui::EndGroup();
		}

		return result;
	}
	
	void prefab_preview_dialog::handle_drag_drop()
	{
		if (!m_dragdrop_prefab_name.empty())
		{
			// target => grid_dialog::drag_drop_target()
			// target => camera_dialog::drag_drop_target()
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				const char* prefab_name = m_dragdrop_prefab_name.c_str();
				const char* prefab_path = m_dragdrop_prefab_path.c_str();

				const bool is_root = !prefab_path || prefab_path[0] == '\0';
				const char* full_path = utils::va(is_root ? "%s%s" : "%s\\%s", is_root ? "" : prefab_path, prefab_name);

				ImGui::SetDragDropPayload("PREFAB_BROWSER_ITEM", full_path, strlen(full_path), ImGuiCond_Once);
				ImGui::Text("Prefab: %s", full_path);
				ImGui::EndDragDropSource();
			}
		}
	}


	bool prefab_preview_dialog::gui()
	{
		imgui::SetNextWindowSize(ImVec2(600.0f, 600.0f), ImGuiCond_Appearing);
		imgui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!imgui::Begin("Prefab Browser##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			imgui::End();
			return false;
		}

		const float thumbnail_size = static_cast<float>(dvars::gui_prefab_browser_img_size->current.integer);

		{
			SPACING(0.0f, 0.0f);
			imgui::SetCursorPosX(imgui::GetCursorPosX() + 4.0f);

			const bool needs_map_closing_popup = game::current_map_filepath && game::current_map_filepath != "unnamed.map"s;
			bool generate_previews_pending = false;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));

			imgui::BeginDisabled(m_current_directory == m_mapsource_directory);
			{
				if (imgui::Button(ICON_FA_CAMERA, ImVec2(imgui::GetFrameHeight(), imgui::GetFrameHeight())))
				{
					generate_previews_pending = true;

					if (needs_map_closing_popup)
					{
						// ImGui::popup_close_map()
						ImGui::OpenPopup("close_map_popup");
					}

				} TT("Generate prefab thumbnails for the current folder");

				imgui::EndDisabled();
			}

			ImGui::PopStyleVar();

			if((generate_previews_pending && !needs_map_closing_popup) || imgui::popup_close_map() == 2)
			{
				prefab_browser_generate_thumbnails = true;
				prefab_browser_generate_thumbnails_folder = m_current_directory;
			}

			imgui::SameLine();

			imgui::BeginDisabled(m_current_directory == m_mapsource_directory); //m_prefab_directory);
			{
				if (imgui::Button("<"))
				{
					m_current_directory = m_current_directory.parent_path();
					update_directory();
				}

				imgui::EndDisabled();
			}

			static ImVec2 addr_bar_mins = ImVec2(0.0f, 0.0f);
			static ImVec2 addr_bar_maxs = ImVec2(1.0f, 1.0f);

			imgui::SameLine();

			imgui::GetWindowDrawList()->AddRectFilled(addr_bar_mins - ImVec2(0.0f, 0.0f), addr_bar_maxs + ImVec2(0.0f, 0.0f), imgui::ColorConvertFloat4ToU32(ImVec4(0.215f, 0.215f, 0.215f, 1.0f)), 4.0f);

			const ImVec4 child_bg = imgui::ToImVec4(dvars::gui_window_bg_color->current.vector) + ImVec4(0.03f, 0.03f, 0.03f, 0.0f);

			imgui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			imgui::PushStyleColor(ImGuiCol_ChildBg, child_bg);
			imgui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			imgui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f));
			imgui::BeginGroup();
			{
				if (imgui::Button("map_source"))
				{
					m_current_directory = m_mapsource_directory;
					update_directory();
				}

				/*if (imgui::Button("prefabs"))
				{
					m_current_directory = m_prefab_directory;
					update_directory();
				}*/

				std::vector<std::pair<std::string, std::filesystem::path>> paths_to_current_dir;

				if (const auto rel_path = std::filesystem::relative(m_current_directory, m_mapsource_directory);
					!rel_path.empty() && rel_path != ".")
				{
					const auto splits = utils::explode(rel_path.string(), '\\');

					int split_num = 0;
					for (const auto& s : splits)
					{
						std::filesystem::path pp;

						if (!paths_to_current_dir.empty())
						{
							pp = paths_to_current_dir[split_num - 1].second;
							pp /= s;
						}
						else
						{
							pp = m_mapsource_directory / s;
						}

						paths_to_current_dir.emplace_back(std::make_pair(s, pp));
						split_num++;
					}
				}

				for (const auto& [name, path] : paths_to_current_dir)
				{
					imgui::SameLine();
					if (imgui::Button(name.c_str()))
					{
						m_current_directory = path;
						update_directory();
					}
				}

				imgui::EndGroup();

				addr_bar_mins = imgui::GetItemRectMin();
				addr_bar_maxs = imgui::GetItemRectMax();
			}

			static float img_size_widget_width = 1;

			imgui::SameLine(imgui::GetWindowWidth() - img_size_widget_width - 8.0f);
			imgui::SetNextItemWidth(40.0f);
			if (imgui::DragInt("##img_size", &dvars::gui_prefab_browser_img_size->current.integer, 1, 64, 256))
			{
				if(dvars::gui_prefab_browser_img_size->current.integer < 64)
				{
					dvars::gui_prefab_browser_img_size->current.integer = 64;
				}
				else if(dvars::gui_prefab_browser_img_size->current.integer > 256)
				{
					dvars::gui_prefab_browser_img_size->current.integer = 256;
				}
			} img_size_widget_width = imgui::GetItemRectSize().x;


			static float filter_widget_width = 1;

			imgui::SameLine(imgui::GetWindowWidth() - img_size_widget_width - filter_widget_width - 12.0f);
			const auto screenpos_prefilter = ImGui::GetCursorScreenPos();
			this->m_filter.Draw("##prefab_filter", 180.0f);
			filter_widget_width = imgui::GetItemRectSize().x;
			const auto screenpos_postfilter = ImGui::GetCursorScreenPos();

			if (!this->m_filter.IsActive())
			{
				ImGui::SetCursorScreenPos(ImVec2(screenpos_prefilter.x + 12.0f, screenpos_prefilter.y + 4.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
				ImGui::TextUnformatted("Filter ..");
				ImGui::PopStyleColor();
				ImGui::SetCursorScreenPos(ImVec2(screenpos_postfilter.x, screenpos_postfilter.y));
			}
			else
			{
				imgui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
				imgui::SetItemAllowOverlap();
				imgui::SameLine(imgui::GetWindowWidth() - img_size_widget_width - 34.0f);
				imgui::SetCursorPosY(imgui::GetCursorPosY() + 2.0f);
				if (ImGui::ButtonEx("x##clear_filter", ImVec2(20.0f, imgui::GetFrameHeightWithSpacing())))
				{
					this->m_filter.Clear();
				}
				imgui::SetCursorPosY(imgui::GetCursorPosY() - 2.0f);
				imgui::PopStyleVar();
			}


			imgui::PopStyleVar(2);

			SPACING(0.0f, 0.0f);

			imgui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			// needs to be this way or ImGui throws an assert when minimizing radiant while having the preference window open
			if (!imgui::BeginChild("##content", ImVec2(0, 0), true))
			{
				imgui::PopStyleColor(3);
				ImGui::EndChild();
				ImGui::End();
				return false;
			}

			imgui::PopStyleColor();

			game::GfxImage* folder_ico;
			if (folder_ico = game::Image_RegisterHandle("folder"); !folder_ico)
			{
				folder_ico = game::Image_RegisterHandle("invalid_texture");
			}

			static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame /*| igTableFlags_BordersOuter*/;

			const float column_width = thumbnail_size + 8.0f;
			int column_count = static_cast<int>(imgui::GetWindowContentRegionMax().x / column_width);
				column_count = column_count < 2 ? 2 : column_count;

			if (imgui::BeginTable("##prefab_table", column_count, flags))
			{
				int folder_num = 0;
				for (const auto& folder : m_curr_dir_folders)
				{
					imgui::TableNextColumn();

					bool updated = false;

					imgui::PushID(folder_num);
					imgui::PushFontFromIndex(E_FONT::BOLD_18PX);

					if (prefab_preview_dialog::image_button(folder_ico, thumbnail_size, 1.0f, folder.c_str()))
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

				if (m_current_directory != m_mapsource_directory)
				{
					for (const auto& [filename, parent_path] : m_curr_dir_files)
					{
						if (this->m_filter.IsActive() && !this->m_filter.PassFilter(filename.c_str()))
						{
							continue;
						}

						imgui::TableNextColumn();

						const auto thumbnail_str_no_ext = get_thumbnail_string(parent_path, filename);

						game::GfxImage* thumbnail = nullptr;

						if (thumbnail = game::Image_FindExisting(thumbnail_str_no_ext.c_str());
							!thumbnail)
						{
							// category 3 can be reloaded but expects an iwi -> rewritten R_ReloadImages -> renderer::on_reload_images
							if (thumbnail = game::Image_Alloc(thumbnail_str_no_ext.c_str(), 66, 2, 0);
								thumbnail)
							{
								const auto jpg_string = "prefab_thumbs\\" + thumbnail_str_no_ext + ".jpg";

								if (!game::R_LoadJpeg(thumbnail, jpg_string.c_str()))
								{
									//game::printf_to_console("failed to load thumbnail '%s'", thumbnail_str.c_str());
								}
							}
						}

						bool update_drag_drop = false;
						const char* filename_str = filename.c_str();

						if (thumbnail && thumbnail->texture.data)
						{
							imgui::PushID(filename_str);
							if (prefab_preview_dialog::image_button(thumbnail, thumbnail_size, 1.0f, filename_str, true))
							{
								const auto current_id = imgui::GetItemID();
								if (!m_dragdrop_id || m_dragdrop_id != current_id)
								{
									m_dragdrop_id = current_id;
									update_drag_drop = true;
								}
							}
							imgui::PopID();
						}
						else
						{
							if (const auto	image = game::Image_RegisterHandle("cycle_xyz");
								image && image->texture.data)
							{
								imgui::PushID(filename_str);
								if (prefab_preview_dialog::image_button(image, thumbnail_size, 0.25f, filename_str, false, 0, ImVec2(0.51f, 0.01f), ImVec2(0.99f, 0.99f)))
								{
									const auto current_id = imgui::GetItemID();
									if (!m_dragdrop_id || m_dragdrop_id != current_id)
									{
										m_dragdrop_id = current_id;
										update_drag_drop = true;
									}
								}
								imgui::PopID();
							}
							else
							{
								if (imgui::Button(filename_str, ImVec2(thumbnail_size, thumbnail_size)))
								{
									const auto current_id = imgui::GetItemID();
									if (!m_dragdrop_id || m_dragdrop_id != current_id)
									{
										m_dragdrop_id = current_id;
										update_drag_drop = true;
									}
								}
							}
						}

						if (update_drag_drop)
						{
							game::printf_to_console("updating drag drop");
							m_dragdrop_prefab_name = filename;
							m_dragdrop_prefab_path = parent_path.string();
						}

						handle_drag_drop();
					}
				}

				imgui::EndTable();
			}

			imgui::PopStyleColor(2);

			imgui::EndChild();
			imgui::End();
		}

		return true;
	}

	void prefab_preview_dialog::on_open()
	{
		const auto egui = GET_GUI(entity_dialog);
		m_mapsource_directory = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

		m_prefab_directory = m_mapsource_directory;
		m_prefab_directory /= "prefabs";

		m_current_directory = m_prefab_directory;

		update_directory();
	}

	void prefab_preview_dialog::on_close()
	{ }

	void prefab_preview_dialog::init()
	{
		components::command::register_command_with_hotkey("prefab_browser"s, [this](auto)
		{
			if (this->is_inactive_tab() && this->is_active())
			{
				this->set_bring_to_front(true);
				return;
			}

			this->toggle();
		});
	}

	void prefab_preview_dialog::register_dvars()
	{
		dvars::gui_prefab_browser_img_size = dvars::register_int(
			/* name		*/ "gui_prefab_browser_img_size",
			/* default	*/ 128,
			/* min		*/ 64,
			/* max		*/ 256,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "size of prefab thumbnails");
	}

	REGISTER_GUI(prefab_preview_dialog);
}
