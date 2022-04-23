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

	void mru_new_item(game::LPMRUMENU* mru, const char* item_str)
	{
		const static uint32_t func_addr = 0x48A2C0;
		__asm
		{
			pushad;
			mov		esi, mru;
			push	item_str;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	void mru_insert_item(game::LPMRUMENU* mru, HMENU menu)
	{
		const static uint32_t func_addr = 0x48A400;
		__asm
		{
			pushad;
			mov		edi, mru;
			push	menu;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	void map_load_from_file(const char* path)
	{
		const static uint32_t func_addr = 0x486680;
		__asm
		{
			pushad;
			mov		ecx, path;
			call	func_addr;
			popad;
		}
	}

	void map_save_file(const char* path /*ecx*/, int is_reg, int save_to_perforce)
	{
		const static uint32_t func_addr = 0x486C00;
		__asm
		{
			pushad;
			mov		ecx, path;
			push	save_to_perforce;
			push	is_reg;
			call	func_addr;
			add		esp, 8;
			popad;
		}
	}

	void mru_add_recent_map(const char* map_str)
	{
		mru_new_item(game::g_qeglobals->d_lpMruMenu, map_str);
		const auto menu = GetSubMenu(GetMenu(game::g_qeglobals->d_hwndMain), 0);
		mru_insert_item(game::g_qeglobals->d_lpMruMenu, menu);
	}

	void file_dialog_frame()
	{
		const auto file = GET_GUI(ggui::file_dialog);
		if (file->is_active() && file->dialog())
		{
			if(const std::string path_out = file->get_path_result();
								!path_out.empty())
			{
				switch(file->get_file_handler())
				{
					case FILE_DIALOG_HANDLER::MAP_LOAD:
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

								map_load_from_file(path_out.c_str());

								// fix stuck left mouse button
								ImGuiIO& io = ImGui::GetIO();
								io.AddMouseButtonEvent(0, false);
							}
							break;
					}

					case FILE_DIALOG_HANDLER::MAP_SAVE:
					{
							std::string file_path = path_out;
							if(!file_path.ends_with(".map"))
							{
								file_path += ".map";
							}

							mru_add_recent_map(file_path.c_str());
							map_save_file(file_path.c_str(), 0, 0);

							// fix stuck left mouse button
							ImGuiIO& io = ImGui::GetIO();
							io.AddMouseButtonEvent(0, false);

							break;
					}
				}
			}
		}
	}
}
