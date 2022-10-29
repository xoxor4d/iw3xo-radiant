#pragma once

namespace ggui
{
	enum GUI_WINDOW_
	{
		GUI_WINDOW_GRID,
		GUI_WINDOW_CAMERA,
	};
}

namespace ggui::context
{
	void xyzcam_general_selection();
	void grouping_menu(game::selbrush_def_t* sb);
}
