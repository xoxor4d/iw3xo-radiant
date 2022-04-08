#include "std_include.hpp"
#include "_ggui.hpp"

namespace ggui
{
	// *
	// | -------------------- Variables ------------------------
	// *


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




    imgui_state_t state = imgui_state_t();
	bool		saved_states_init = false;
	
	ImGuiID		dockspace_outer_left_node;
	bool		reset_dockspace = false;
	bool		mainframe_menubar_enabled = false;
	float		menubar_height;
	
	ImVec2		toolbar_pos;
	ImVec2		toolbar_size;
	ImGuiAxis	toolbar_axis = ImGuiAxis_X;
	bool		toolbar_reset = false;
	ImGuiID		toolbar_dock_top;
	ImGuiID		toolbar_dock_left;

	//model_selector_s rtt_model_preview;

	std::vector<commandbinds> cmd_hotkeys;

	// * cmainframe::on_keydown()
	// * ggui::hotkeys::load_commandmap()
	// add additional radiant-builtins
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
		if(ggui::toolbar_axis == ImGuiAxis_X)
		{
			return ImVec2(5.0f, 33.0f + ggui::toolbar_size.y + 5.0f);
		}

		return ImVec2(ggui::toolbar_size.x + 10.0f, 33.0f);
	}

	void set_next_window_initial_pos_and_constraints(ImVec2 mins, ImVec2 initial_size, ImVec2 overwrite_pos)
	{
		ImGui::SetNextWindowSizeConstraints(mins, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(initial_size, ImGuiCond_FirstUseEver);

		if(overwrite_pos.x == 0 && overwrite_pos.y == 0)
		{
			ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		}
		else
		{
			ImGui::SetNextWindowPos(overwrite_pos, ImGuiCond_FirstUseEver);
		}
	}

	bool cz_context_ready()
	{
		return ggui::state.czwnd.context_initialized;
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

	// ^
	// handles "window_hovered" for widgets drawn over rtt windows (needs to be called after every widget)
	bool rtt_handle_windowfocus_overlaywidget(ggui::render_to_texture_window_s* wnd)
	{
		return rtt_handle_windowfocus_overlaywidget(&wnd->window_hovered);
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

	// ^
	// redraw tabbar triangle -> blocking mouse input for that area so one can actually use the triangle to unhide the tabbar
	void redraw_undocking_triangle(ImGuiWindow* wnd, ggui::render_to_texture_window_s* rtt)
	{
		redraw_undocking_triangle(wnd, &rtt->window_hovered);
	}

	void dragdrop_overwrite_leftmouse_capture()
	{
		get_rtt_camerawnd()->capture_left_mousebutton = true;
		get_rtt_gridwnd()->capture_left_mousebutton = true;
	}

	void dragdrop_reset_leftmouse_capture()
	{
		get_rtt_camerawnd()->capture_left_mousebutton = false;
		get_rtt_gridwnd()->capture_left_mousebutton = false;
	}
	
}
