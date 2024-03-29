#include "std_include.hpp"
#include "_ggui.hpp"

namespace ggui
{
	std::vector<std::unique_ptr<ggui_module>>* loader::modules_ = nullptr;

	void loader::register_gui(std::unique_ptr<ggui_module>&& module_)
	{
		if (!modules_)
		{
			modules_ = new std::vector<std::unique_ptr<ggui_module>>();
			atexit(destroy_modules);
		}

		modules_->push_back(std::move(module_));
	}

	void loader::destroy_modules()
	{
		if (!modules_)
		{
			return;
		}

		delete modules_;
		modules_ = nullptr;
	}

	// *
	// | -------------------- Variables ------------------------
	// *

	bool		m_init_saved_states = false;

	bool		m_ggui_initialized = false;
	bool		m_ggui_second_frame = false;

	ImGuiContext* m_ggui_context = nullptr;

	bool		m_dockspace_initiated = false;
	bool		m_dockspace_reset = false;
	ImGuiID		m_dockspace_outer_left_node;
	bool		mainframe_menubar_enabled = false; // is stock menubar visible? (also used for asm stubs)

	bool		m_demo_menu_state = false;
	
	std::vector<commandbinds> cmd_hotkeys;

	// * cmainframe::on_keydown()
	// * ggui::hotkeys::load_commandmap()
	//   add additional radiant-builtins
	std::vector<game::SCommandInfo> cmd_addon_hotkeys_builtin
	{
		{ "LockX", 0, 0, 0x802E },
		{ "LockY", 0, 0, 0x802F },
		{ "LockZ", 0, 0, 0x8030 },
	};

	// bind commands to keys (components::command::execute())
	std::vector<game::SCommandInfoHotkey> cmd_addon_hotkeys;
	
	// *
	// | -------------------- Functions ------------------------
	// *

	ImVec2 get_initial_window_pos()
	{
		const auto tb = GET_GUI(ggui::toolbar_dialog);
		if(tb->m_toolbar_axis == ImGuiAxis_X)
		{
			return { 5.0f, 33.0f + tb->m_toolbar_size.y + 5.0f };
		}

		return { tb->m_toolbar_size.x + 10.0f, 33.0f };
	}

	void set_next_window_initial_pos_and_constraints(ImVec2 mins, ImVec2 initial_size, ImVec2 overwrite_pos)
	{
		ImGui::SetNextWindowSizeConstraints(mins, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);

		if(overwrite_pos.x == 0.0f && overwrite_pos.y == 0.0f)
		{
			ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		}
		else
		{
			ImGui::SetNextWindowPos(overwrite_pos, ImGuiCond_FirstUseEver);
		}
	}

	bool is_ggui_initialized()
	{
		return ggui::m_ggui_initialized;
	}

	// ends with ggui::context_menu_style_end
	void context_menu_style_begin()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 7.0f));

		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.11f, 0.11f, 0.11f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.224f, 0.224f, 0.224f, 1.0f));
	}

	// starts with ggui::context_menu_style_begin
	void context_menu_style_end()
	{
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	// handles "window_hovered" for widgets drawn over rtt windows (needs to be called after every widget)
	bool rtt_handle_windowfocus_overlaywidget(bool* gui_hover_state)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
		{
			*gui_hover_state = false;
			return true;
		}

		return false;
	}


	// redraw tabbar triangle -> blocking mouse input for that area so one can actually use the triangle to unhide the tabbar
	void redraw_undocking_triangle(ImGuiWindow* wnd, bool* gui_hover_state)
	{
		if (wnd->DockIsActive && wnd->DockNode->IsHiddenTabBar() && !wnd->DockNode->IsNoTabBar())
		{
			const float unhide_sz_draw = ImFloor(ImGui::GetFontSize() * 0.70f);
			const float unhide_sz_hit = ImFloor(ImGui::GetFontSize() * 0.55f);
			const ImVec2 p = wnd->DockNode->Pos;

			ImGui::InvisibleButton("##unhide_hack", ImVec2(unhide_sz_hit, unhide_sz_hit));

			const bool hovered = ggui::rtt_handle_windowfocus_overlaywidget(gui_hover_state);


			if (dvars::gui_rtt_padding_enabled && !dvars::gui_rtt_padding_enabled->current.enabled)
			{
				const auto col_hover = ImGui::GetColorU32(ImGuiCol_ButtonActive);
				const auto col_bg = ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));

				ImGui::GetWindowDrawList()->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), hovered ? col_hover : col_bg);

				// wnd is not actually the window we want to draw the triangle on, its the childwindow where we draw the rtt image ..
				// wnd->DrawList->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), col);

				// always on top
				// ImGui::GetForegroundDrawList()->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), col);
			}

			//ImGui::Indent(8.0f);
			//ImGui::Text("Hovered Triangle? %d", hovered);
		}
	}

	void dragdrop_overwrite_leftmouse_capture()
	{
		GET_GUI(ggui::camera_dialog)->rtt_set_lmb_capturing(true);
		GET_GUI(ggui::grid_dialog)->rtt_set_lmb_capturing(true);
	}

	void dragdrop_reset_leftmouse_capture()
	{
		GET_GUI(ggui::camera_dialog)->rtt_set_lmb_capturing(false);
		GET_GUI(ggui::grid_dialog)->rtt_set_lmb_capturing(false);
	}

	

	void mru_add_recent_map(const char* map_str)
	{
		game::mru_new_item(game::g_qeglobals->d_lpMruMenu, map_str);
		const auto menu = GetSubMenu(GetMenu(game::g_qeglobals->d_hwndMain), 0);
		game::mru_insert_item(game::g_qeglobals->d_lpMruMenu, menu);
	}

	bool wait_frame = false;

	void file_dialog_frame()
	{
		//if(wait_frame)
		//{
		//	auto sab = game::g_selected_brushes();
		//	auto sabn = game::g_selected_brushes_next();

		//	auto ab = game::g_active_brushes();
		//	auto abn = game::g_active_brushes_next();

		//	int x = 1;

		//	// also update cached prefabs?
		//	cdeclcall(void, 0x42BF70); // CMainFrame::OnPrefabEnter
		//	cdeclcall(void, 0x42BF80); // CMainFrame::OnPrefabLeave

		//	wait_frame = false;
		//}

		const auto file = GET_GUI(ggui::file_dialog);
		if (file->is_active() && file->dialog() || file->was_canceled())
		{
			const std::string path_out = file->get_path_result();
			const int handler = file->get_file_handler();

			if(!file->do_callback())
			{
				switch (handler)
				{
					case CUSTOM:
						// do nothing because user is using a custom callback
						break;

					case MAP_LOAD:
					{
						if (!file->was_canceled() && !path_out.empty())
						{
							// checks for unsaved changes or if inside prefab 
							if (!utils::hook::call<bool(__cdecl)()>(0x489D90)()
								// loose changes modal 
								|| utils::hook::call<bool(__cdecl)()>(0x49A030)())
							{
								// leave all prefabs and go back to stacklevel 0
								utils::hook::call<void(__cdecl)()>(0x489D50)();

								mru_add_recent_map(path_out.c_str());

								// Pointfile_Clear
								utils::hook::call<void(__cdecl)()>(0x410600)();

								game::map_load_from_file(path_out.c_str());

								// fix stuck left mouse button
								ImGuiIO& io = ImGui::GetIO();
								io.AddMouseButtonEvent(0, false);
							}
						}

						break;
					}

					case MAP_SAVE:
					{
						if (!file->was_canceled() && !path_out.empty())
						{
							std::string file_path = path_out;
							if (!file_path.ends_with(".map"))
							{
								file_path += ".map";
							}

							mru_add_recent_map(file_path.c_str());
							game::map_save_file(file_path.c_str(), 0, 0);

							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);
						}

						break;
					}

					case MAP_EXPORT:
					case MISC_PREFAB_CREATE:
					{
						if (!file->was_canceled() && !path_out.empty())
						{
							std::string file_path = path_out;
							if (!file_path.ends_with(".map"))
							{
								file_path += ".map";
							}

							game::map_write_selection(file_path.c_str());

							if(handler == MISC_PREFAB_CREATE)
							{
								const auto ent = GET_GUI(ggui::entity_dialog);

								// delete selection
								game::Select_Delete();

								// create prefab at 0, 0, 0
								game::CreateEntityBrush(0, 0, cmainframe::activewnd->m_pXYWnd);

								// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
								g_block_radiant_modeldialog = true;
								game::CreateEntityFromName("misc_prefab");
								g_block_radiant_modeldialog = false;

								const std::string replace_path = "map_source\\";
								const std::size_t pos = file_path.find(replace_path) + replace_path.length();

								std::string loc_filepath = file_path.substr(pos);
								utils::replace(loc_filepath, "\\", "/");

								ent->add_prop("model", loc_filepath.c_str());
								ent->add_prop("origin", "0 0 0");

								auto edit_ent = game::g_edit_entity();
								++edit_ent->version;
								edit_ent->modelClass = nullptr;
								edit_ent->brushes.oprev->unk01 = 0;

								components::exec::on_gui_once([]
								{
									// enter and leave prefab on next frame to update the prefab incase it was modified
									// this does not work in the frame the prefab was spawned in

									game::Prefab_Enter();
									game::Prefab_Leave();
								});
							}

							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);
						}

						break;
					}

					case MISC_MODEL:
					case MISC_MODEL_CHANGE:
					case MISC_PREFAB:
					case MISC_PREFAB_CHANGE:
					{
						const bool is_model = (handler == MISC_MODEL || handler == MISC_MODEL_CHANGE);
						const bool is_changing = (handler == MISC_MODEL_CHANGE || handler == MISC_PREFAB_CHANGE);

						if (!file->was_canceled() && !path_out.empty())
						{
							const std::string replace_path = is_model ? "raw\\xmodel\\" : "map_source\\";
							const std::size_t pos = path_out.find(replace_path) + replace_path.length();

							std::string loc_filepath = path_out.substr(pos);
							utils::replace(loc_filepath, "\\", "/");

							const auto ent = GET_GUI(ggui::entity_dialog);
							ent->add_prop("model", loc_filepath.c_str());

							if (!is_changing)
							{
								auto edit_ent = game::g_edit_entity();
								++edit_ent->version;
								edit_ent->modelClass = nullptr;
								edit_ent->brushes.oprev->unk01 = 0;
							}


							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);

							break;
						}
						else
						{
							if (!is_changing)
							{
								cdeclcall(void, 0x425690);
							}
						}
					}

					case FX_CHANGE:
					{
						if (!file->was_canceled() && !path_out.empty())
						{
							const std::string replace_path = "raw\\fx\\";
							const std::size_t pos = path_out.find(replace_path) + replace_path.length();

							std::string loc_filepath = path_out.substr(pos);
							utils::replace(loc_filepath, "\\", "/");
							utils::erase_substring(loc_filepath, ".efx"s);

							const auto ent = GET_GUI(ggui::entity_dialog);
							ent->add_prop("fx", loc_filepath.c_str());


							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);

							break;
						}
					}

					case FX_EDITOR_DEF:
					{
						// aaaaaaa
					}

					case D3DBSP_LOAD:
					{
						if (!file->was_canceled() && !path_out.empty())
						{
							const std::string replace_path = "raw\\";
							const std::size_t pos = path_out.find(replace_path) + replace_path.length();

							std::string loc_filepath = path_out.substr(pos);
							utils::replace(loc_filepath, "\\", "/");

							if(components::d3dbsp::radiant_load_bsp(loc_filepath.c_str()))
							{
								dvars::set_bool(dvars::r_draw_bsp, true);
							}

							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);

						}

						break;
					}
				}
			}

			if(file->was_canceled())
			{
				file->reset(true);
				file->fix_on_close();
			}
		}
	}
}
