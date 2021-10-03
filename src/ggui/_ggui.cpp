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
	
}