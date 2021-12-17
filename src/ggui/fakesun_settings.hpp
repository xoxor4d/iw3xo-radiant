#pragma once
#include "_ggui.hpp"

namespace ggui::fakesun_settings
{
	extern float	sun_dir[3];
	extern float	sun_diffuse[3];
	extern float	sun_specular[4];
	extern float	material_specular[4];
	extern float	ambient[4];
	
	void	menu(ggui::imgui_context_menu& menu);

}
