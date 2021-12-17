#pragma once
#include "_ggui.hpp"

namespace ggui::preferences
{
	extern int		dev_num_01;
	extern float	dev_vec_01[4];
	extern float	dev_color_01[4];
	
	void	menu(ggui::imgui_context_menu& menu);
	void	register_dvars();
	void	hooks();
	
}
