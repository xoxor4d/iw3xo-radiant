#include "std_include.hpp"

namespace ggui
{
	// *
	// | -------------------- Variables ------------------------
	// *

    imgui_state_t state = imgui_state_t();

	ImGuiID		dockspace_sidebar_id;
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

	rtt_camera_window_s rtt_camerawnd = rtt_camera_window_s();
	rtt_grid_window_s rtt_gridwnd = rtt_grid_window_s();
	
	// *
	// | -------------------- Functions ------------------------
	// *

	ImVec2 get_initial_window_pos()
	{
		return ImVec2(5.0f, ggui::toolbar_pos.y + ggui::toolbar_size.y + 5.0f);
	}

	bool camera_context_ready()
	{
		return ggui::state.ccamerawnd.context_initialized;
	}
	
	bool cxy_context_ready()
	{
		return ggui::state.cxywnd.context_initialized;
	}

	bool layered_context_ready()
	{
		return ggui::state.clayeredwnd.context_initialized;
	}

	bool cz_context_ready()
	{
		return ggui::state.czwnd.context_initialized;
	}
	
	bool all_contexts_ready()
	{
		if (ggui::state.ccamerawnd.context_initialized
			 && ggui::state.cxywnd.context_initialized)
		{
			return true;
		}

		return false;
	}
	
}