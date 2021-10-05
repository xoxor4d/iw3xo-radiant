#include "std_include.hpp"

namespace ggui
{
	// *
	// | -------------------- Variables ------------------------
	// *

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

	std::vector<commandbinds> cmd_hotkeys;

	//render_to_texture_window_s rtt_camerawnd = render_to_texture_window_s();
	//render_to_texture_window_s rtt_gridwnd = render_to_texture_window_s();
	//render_to_texture_window_s rtt_texwnd = render_to_texture_window_s();
	
	// *
	// | -------------------- Functions ------------------------
	// *

	ImVec2 get_initial_window_pos()
	{
		return ImVec2(5.0f, ggui::toolbar_pos.y + ggui::toolbar_size.y + 5.0f);
	}

	bool layered_context_ready()
	{
		return ggui::state.clayeredwnd.context_initialized;
	}

	bool cz_context_ready()
	{
		return ggui::state.czwnd.context_initialized;
	}

	// handles "window_hovered" for widgets drawn over rtt windows
	// needs to be called after every widget
	bool rtt_handle_windowfocus_overlaywidget(ggui::render_to_texture_window_s* wnd)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
		{
			wnd->window_hovered = false;
			return true;
		}

		return false;
	}

	// redraw tabbar triangle -> blocking mouse input for that area so one can actually use the triangle to unhide the tabbar
	void FixDockingTabbarTriangle(ImGuiWindow* wnd, ggui::render_to_texture_window_s* rtt)
	{
		if (wnd->DockIsActive && wnd->DockNode->IsHiddenTabBar() && !wnd->DockNode->IsNoTabBar())
		{
			const float unhide_sz_draw = ImFloor(ImGui::GetFontSize() * 0.70f);
			const float unhide_sz_hit = ImFloor(ImGui::GetFontSize() * 0.55f);
			const ImVec2 p = wnd->DockNode->Pos;

			ImGui::InvisibleButton("##unhide_hack", ImVec2(unhide_sz_hit, unhide_sz_hit));

			const bool hovered = ggui::rtt_handle_windowfocus_overlaywidget(rtt);
			const auto col = ImGui::GetColorU32(hovered ? ImGuiCol_ButtonActive : ImGuiCol_Button);

			ImGui::GetForegroundDrawList()->AddTriangleFilled(p, p + ImVec2(unhide_sz_draw, 0.0f), p + ImVec2(0.0f, unhide_sz_draw), col);

			//ImGui::Indent(8.0f);
			//ImGui::Text("Hovered Triangle? %d", hovered);
		}
	}
	
}