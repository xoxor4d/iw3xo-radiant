#pragma once
#include "_ggui.hpp"

namespace ggui::preferences
{
	extern int		dev_num_01;
	extern float	modelpreview_sun_dir[3];
	extern float	modelpreview_sun_diffuse[3];
	extern float	modelpreview_sun_specular[4];
	extern float	modelpreview_material_specular[4];
	extern float	modelpreview_ambient[4];
	
	void	menu(ggui::imgui_context_menu& menu);
	void	register_dvars();
	void	hooks();
	
}
